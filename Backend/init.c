/* init cgroup & chroot environment
 * should be run only once at the beginning
 */

#include "util.h"
#include "init.h"

char tmpDir[] = "/tmp/ojs-XXXXXX";

void cleanup() {
    rmdir(CGROUP_DIR "cpuacct/" CNAME);
    rmdir(CGROUP_DIR "memory/" CNAME);
    rmdir(CGROUP_DIR "pids/" CNAME);
    system("umount /tmp/ojs-*/*");
    system("umount /tmp/ojs-*/dev/*");
    system("umount /tmp/ojs-*/usr/*");
    system("rm -rf /tmp/ojs-*");
    exit(0);
}

int main(int argc, char **argv) {
    if (!isRootUser()) {
        fprintf(stderr, "This program requires root user.\n");
        exit(-1);
    }
    if (argc == 2 && strcmp(argv[1], "cleanup") == 0) {
        cleanup();
    }
    int res;
    res = mkdir(CGROUP_DIR "cpuacct/" CNAME, MODE0);
    if (res == -1 && errno != EEXIST)
        errorExit(CGERR);
    res = mkdir(CGROUP_DIR "memory/" CNAME, MODE0);
    if (res == -1 && errno != EEXIST)
        errorExit(CGERR);
    res = mkdir(CGROUP_DIR "pids/" CNAME, MODE0);
    if (res == -1 && errno != EEXIST)
        errorExit(CGERR);
    
    char *tmp = mkdtemp(tmpDir);
    if (tmp == NULL) {
        errorExit(TPERR);
    }
    // reference: jd4
    chdir(tmp);
    mkdir("proc", MODE0);
    mkdir("dev", MODE0);
    mkdir("tmp", MODE0);
    mount("proc", "proc", "proc", MS_NOSUID, NULL);
    mknod("dev/null", 0666, S_IFCHR);
    mknod("dev/urandom", 0666, S_IFCHR);
    mount("/dev/null", "dev/null", "", MS_BIND | MS_NOSUID, NULL);
    mount("/dev/urandom", "dev/urandom", "", MS_BIND | MS_NOSUID, NULL);
    mount("tmp", "tmp", "tmpfs", MS_NOSUID, "size=16m,nr_inodes=4k");

    mkdir("bin", MODE0);
    mkdir("lib", MODE0);
    mkdir("lib64", MODE0);
    mkdir("usr", MODE0);
    mkdir("usr/bin", MODE0);
    mkdir("usr/include", MODE0);
    mkdir("usr/lib", MODE0);
    mkdir("usr/lib64", MODE0);
    mkdir("usr/libexec", MODE0);
    mkdir("usr/share", MODE0);
    bindMountHelper("/bin", "bin");
    bindMountHelper("/lib", "lib");
    bindMountHelper("/lib64", "lib64");
    bindMountHelper("/usr/bin", "usr/bin");
    bindMountHelper("/usr/include", "usr/include");
    bindMountHelper("/usr/lib", "usr/lib");
    // bindMountHelper("/usr/lib64", "usr/lib64");
    // bindMountHelper("/usr/libexec", "usr/libexec");
    bindMountHelper("/usr/share", "usr/share");
    printf("The tmp dir: %s\n", tmp);
    return 0;
}