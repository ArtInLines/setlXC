#ifdef __WIN32
#include <windows.h>
#else // Linux
#include <sys/wait.h>
#include <unistd.h>
#endif // __WIN32
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#define AIL_ALL_IMPL
#include "deps/ail/ail.h"
#define AIL_FS_IMPL
#include "deps/ail/ail_fs.h"
#define AIL_SV_IMPL
#include "deps/ail/ail_sv.h"

#define PIPE_SIZE 2048
#ifndef MAX_RUNNING_THREADS
#define MAX_RUNNING_THREADS 4
#endif // MAX_RUNNING_THREADS
static u32 cur_running_threads = 0;
pthread_mutex_t running_threads_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef __WIN32
#define DIR_SEP     "\\"
#define ALT_DIR_SEP "/"
#else // Linux
#define DIR_SEP     "/"
#define ALT_DIR_SEP "\\"
#endif // __WIN32

// @Cleanup: I leak a bunch of memory here, which is fine for now, cause it's really just a small script, but I should probably clean that up someday, especially if I create more tests

typedef struct {
    AIL_DA(char) out;
    AIL_SV file;
} Testcase;
AIL_DA_INIT(Testcase);

typedef struct {
    pthread_t t;    // The current thread
    bool      done; // Whether the thread is done with its computation
    bool      succ; // Whether the test was successfull
    bool      err;  // Whether an error occured
    AIL_SV    file; // The path for the file to test
    AIL_SV    out;  // Received output
    AIL_SV    exp;  // Expected output
} Thread;

void print_thread(Thread t)
{
    printf("Thread: {\n");
    printf("  t: %lld,\n", t.t);
    printf("  done: %d,\n", t.done);
    printf("  succ: %d,\n", t.succ);
    printf("  err: %d,\n", t.err);
    printf("  file: %s,\n", ail_sv_copy_to_cstr(t.file));
    printf("  out: %s,\n", ail_sv_copy_to_cstr(t.out));
    printf("  exp: %s,\n", ail_sv_copy_to_cstr(t.exp));
    printf("}\n");
}

#ifdef __WIN32
void* run(void *arg)
{
    Thread *in   = (Thread *)arg;
    bool running = false;
    while (!running) {
        pthread_mutex_lock(&running_threads_mutex);
        if (cur_running_threads < MAX_RUNNING_THREADS) {
            cur_running_threads++;
            running = true;
        }
        pthread_mutex_unlock(&running_threads_mutex);
    }
    char *file_str = ail_sv_copy_to_cstr(in->file);
    // printf("starting to test: %s\n", file_str);

    AIL_DA(char) out = ail_da_new(char);
    char buf[PIPE_SIZE] = {0};
    HANDLE pipe_read;
    HANDLE pipe_write;
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    if (!CreatePipe(&pipe_read, &pipe_write, &saAttr, 0)) {
        in->out = ail_sv_from_cstr("Could not establish pipe to child process.");
        goto err;
    }

    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (siStartInfo.hStdOutput == INVALID_HANDLE_VALUE) {
        in->out = ail_sv_from_cstr("Could not handle to child's stdout.");
        goto err;
    }
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if (siStartInfo.hStdError == INVALID_HANDLE_VALUE) {
        in->out = ail_sv_from_cstr("Could not handle to child's stderr.");
        goto err;
    }
    siStartInfo.hStdOutput = pipe_write;
    siStartInfo.hStdError  = pipe_write;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    char cmd_buffer[1028] = {0};
    snprintf(cmd_buffer, 16 + in->file.len, "run.bat %s", file_str);
    if (!CreateProcess(NULL, cmd_buffer, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
        in->out  = ail_sv_from_cstr("Could not create child process.");
        CloseHandle(pipe_write);
        CloseHandle(pipe_read);
        goto err;
    }

    if (WaitForSingleObject(piProcInfo.hProcess, INFINITE) == WAIT_FAILED) {
        in->out  = ail_sv_from_cstr("Could not wait for child process.");
        CloseHandle(piProcInfo.hProcess);
        goto err;
    }

    DWORD nBytesRead;
    if (ReadFile(pipe_read, buf, PIPE_SIZE, &nBytesRead, 0)) {
        ail_da_pushn(&out, buf, strlen(buf));
    }

    const AIL_SV rn = ail_sv_from_cstr("\r\n");
    const AIL_SV n  = ail_sv_from_cstr("\n");
    // printf("out: %s\n", ail_sv_copy_to_cstr(ail_sv_replace(ail_sv_trim(ail_sv_from_da(out)), rn, n)));
    // print_thread(*in);
    // printf("done testing file: %s\n", file_str);
    in->out  = ail_sv_replace(ail_sv_trim(ail_sv_from_da(out)), rn, n);
    // printf("exp: %s\n", ail_sv_copy_to_cstr(in->exp));
    in->exp  = ail_sv_replace(ail_sv_trim(in->exp), rn, n);
    in->succ = ail_sv_eq(in->out, in->exp);
    in->done = true;

    free(file_str);
    CloseHandle(pipe_read);
    CloseHandle(piProcInfo.hThread);
    FlushFileBuffers(pipe_write);
    CloseHandle(pipe_write);
    CloseHandle(piProcInfo.hProcess);

    pthread_mutex_lock(&running_threads_mutex);
    cur_running_threads--;
    pthread_mutex_unlock(&running_threads_mutex);
    ail_da_free(&out);
    return NULL;
err:
    pthread_mutex_lock(&running_threads_mutex);
    cur_running_threads--;
    pthread_mutex_unlock(&running_threads_mutex);
    ail_da_free(&out);
    in->err  = true;
    in->done = true;
    return NULL;
}
#else
void* run(void *arg)
{
    Thread *in = (Thread *)arg;
    bool running = false;
    while (!running && cur_running_threads < MAX_RUNNING_THREADS) {
        pthread_mutex_lock(&running_threads_mutex);
        if (cur_running_threads < MAX_RUNNING_THREADS) {
            cur_running_threads++;
            running = true;
        }
        pthread_mutex_unlock(&running_threads_mutex);
    }

    AIL_DA(char) out = ail_da_new(char);
    int pipefd[2];
    if (pipe(pipefd) != -1) {
        in->out  = ail_sv_from_cstr("Could not establish pipe to child process.");
        goto err;
    }

    pid_t child = fork();
    if (child < 0) {
        in->out = ail_sv_from_cstr("Could not create child process.");
        goto err;
    } else if (child == 0) { // Run by child
        close(pipefd[0]);
        char buf[PIPE_SIZE] = {0};
        execlp("run.bat", ail_sv_copy_to_cstr(in->file), (char *)0);
        while (read(stdout, buf, PIPE_SIZE) != EOF) {
            ail_da_pushn(&out, buf, strlen(buf));
            memset(buf, 0, PIPE_SIZE);
        }
        while (read(stdout, buf, PIPE_SIZE) != EOF) {
            ail_da_pushn(&out, buf, strlen(buf));
            memset(buf, 0, PIPE_SIZE);
        }
        write(pipefd[1], out.data, out.len);
        close(pipefd[1]); // Reader will see EOF
    } else { // Run by parent
        close(pipefd[1]);
        int wsatatus = 0;
        char buf[PIPE_SIZE] = {0};
        while (read(pipefd[0], buf, PIPE_SIZE) != EOF) {
            ail_da_pushn(&out, buf, strlen(buf));
            memset(buf, 0, PIPE_SIZE);
        }
        int wstatus = 0;
        if (waitpid(child, &wstatus, 0) < 0) {
            in->out = ail_sv_from_cstr("Failed to wait for child process.");
            goto err;
        }

        const AIL_SV rn = ail_sv_from_cstr("\r\n");
        const AIL_SV n  = ail_sv_from_cstr("\n");
        in->out  = ail_sv_replace(ail_sv_trim(ail_sv_from_da(out)), rn, n);
        in->exp  = ail_sv_replace(ail_sv_trim(in->exp), rn, n);
        in->succ = ail_sv_eq(in->out, in->exp);
        in->done = true;
    }

    pthread_mutex_lock(&running_threads_mutex);
    cur_running_threads--;
    pthread_mutex_unlock(&running_threads_mutex);
    ail_da_free(&out);
    return NULL;
err:
    pthread_mutex_lock(&running_threads_mutex);
    cur_running_threads--;
    pthread_mutex_unlock(&running_threads_mutex);
    ail_da_free(&out);
    in->err  = true;
    in->done = true;
    return NULL;
}
#endif // __WIN32

void print_usage(void)
{
    printf("Usage:\n");
    printf("test [-v|--verbose] [-r|--record] [-h|--help] [<files>]\n");
    printf("  The order in which you supply the arguments does not matter.\n");
    printf("  You can provide as many files as you wish to test.\n");
    printf("   If you provide no files, all files listed in tests.txt will be tested.\n");
    printf("  If you provide the path to directory, all files from that directory, that are listed in tests.txt will be tested.\n");
    printf("  -v, --verbose = Displays the expected and received output on failure.\n");
    printf("  -r, --record  = Overwrites the expected result with the received result. New tests can also be added this way.\n");
    printf("  -h, --help    = Displays this help message and ends the program.\n");
}

int main(int argc, char **argv)
{
    float start = ((float)clock()) / ((float)CLOCKS_PER_SEC);

    bool verbose = 0;
    bool record  = 0;
    AIL_DA(AIL_SV) files = ail_da_new(AIL_SV);
    for (i32 i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            // Parse option
            if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--record") == 0) {
                record = true;
                fprintf(stderr, "\033[31mRecording is not yet implemented.\033[0m\n");
                return 1;
            } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                print_usage();
                return 0;
            } else {
                fprintf(stderr, "\033[31mUnknown command line option %s\033[0m\n", argv[i]);
                print_usage();
                return 1;
            }
        } else {
            // parse filename
            ail_da_push(&files, ail_sv_replace(ail_sv_from_cstr(argv[i]), ail_sv_from_cstr(ALT_DIR_SEP), ail_sv_from_cstr(DIR_SEP)));
        }
    }
    bool test_all = files.len == 0;

    u64 fsize;
    char *test_file = ail_fs_read_file("tests.txt", &fsize);
    AIL_DA(AIL_SV) path = ail_da_with_cap(AIL_SV, 8);
    AIL_DA(AIL_SV) test_lines = ail_sv_split_lines(ail_sv_from_parts(test_file, fsize), true);
    AIL_DA(Testcase) tests = ail_da_new(Testcase);
    u32 cur_test_idx = 0;
    for (u32 i = 0; i < test_lines.len; i++) {
        if (test_lines.data[i].str[0] == '#') {
            cur_test_idx = tests.len;
            u32 j = 1;
            while (test_lines.data[i].str[j] == '#' && j < test_lines.data[i].len) j++;
            AIL_SV part = ail_sv_trim(ail_sv_offset(test_lines.data[i], j));
            u32 depth = j - 1;
            if (path.len > depth) {
                path.len = depth;
                if (part.len > 0) ail_da_push(&path, part);
                Testcase test = {
                    .out  = ail_da_new_empty(char),
                    .file = ail_sv_join(path.data, path.len, ail_sv_from_cstr(DIR_SEP)),
                };
                ail_da_push(&tests, test);
            } else if (path.len + 1 < depth) {
                fprintf(stderr, "\033[31mError in parsing tests.txt - Missing parent directory in line %d.\033[0m\n", i);
                return 1;
            } else {
                if (part.len > 0) ail_da_push(&path, part);
            }
        } else {
            if (cur_test_idx >= tests.len) {
                Testcase cur_test = {
                    .out  = ail_da_new(char),
                    .file = ail_sv_join(path.data, path.len, ail_sv_from_cstr(DIR_SEP)),
                };
                cur_test_idx = tests.len;
                ail_da_push(&tests, cur_test);
            }
            ail_da_pushn(&tests.data[cur_test_idx].out, test_lines.data[i].str, test_lines.data[i].len);
            ail_da_push(&tests.data[cur_test_idx].out, '\n');
        }
    }

    Thread *threads = malloc(sizeof(Thread) * tests.len);
    u32 total = 0;
    for (u32 i = 0; i < tests.len; i++) {
        bool to_test = test_all;
        for (u32 j = 0; !to_test && j < files.len; j++) {
            if (ail_sv_ends_with_char(files.data[j], *DIR_SEP)) {
                to_test = ail_sv_starts_with(tests.data[i].file, files.data[j]);
            } else {
                to_test = ail_sv_eq(tests.data[i].file, files.data[j]);
            }
        }
        if (to_test) {
            // printf("to test: %s\n", ail_sv_copy_to_cstr(tests.data[i].file));
            threads[total] = (Thread) {
                .done = false,
                .err  = false,
                .succ = false,
                .file = tests.data[i].file,
                .exp  = ail_sv_from_da(tests.data[i].out),
                .out  = ail_sv_from_parts("", 0),
            };
            // printf("expected: %s\n", ail_sv_copy_to_cstr(threads[0].exp));
            pthread_create(&threads[total].t, NULL, &run, &threads[total]);
            total++;
        }
    }

    // Loop until all threads are done:
loop:
    for (u32 i = 0; i < total; i++) {
        if (!threads[i].done) goto loop;
    }

    u32 succ = 0;
    // getting here means all threads are done:
    for (u32 i = 0; i < total; i++) {
        if (threads[i].succ) {
            succ++;
            printf("\033[32mTest %s successful :)\033[0m\n", ail_sv_copy_to_cstr(threads[i].file));
        } else {
            printf("\033[31mTest %s failed :(\033[0m\n", ail_sv_copy_to_cstr(threads[i].file));
            if (verbose) {
                printf("  Expected:\n");
                AIL_DA(AIL_SV) exp = ail_sv_split_lines(threads[i].exp, true);
                printf("    %s\n", ail_sv_copy_to_cstr(ail_sv_join(exp.data, exp.len, ail_sv_from_cstr("\n    "))));
                printf("  Recevied:\n");
                AIL_DA(AIL_SV) out = ail_sv_split_lines(threads[i].out, true);
                printf("    %s\n", ail_sv_copy_to_cstr(ail_sv_join(out.data, out.len, ail_sv_from_cstr("\n    "))));
            }
            if (record) {
                AIL_TODO();
            }
        }
    }
    if (succ == total) {
        printf("\033[032mAll %d out of %d test successful \\o/\033[0m\n", succ, total);
    } else {
        printf("%d out of %d tests successful\n", succ, total);
    }

    float end = ((float)clock()) / ((float)CLOCKS_PER_SEC);
    printf("Time: %fs\n", end - start);

    return 0;
}