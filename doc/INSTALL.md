
התקנה עם ConnectX-3
=============================

על מנת ש-VMA יפעל יש להוסיף את השורה:

    options mlx4_core log_num_mgm_entry_size=-1

לקובץ
&lrm;/etc/modprobe.d/mlx4.conf

ולהפעיל את הפקודה:

    update-initramfs

יש להוסיף את המודולים הבאים לרשימת המודולים הנטענים עם עליית המערכת
בקובץ

&lrm;/etc/modules-load.d/rdma.conf:

    mlx4_core
    mlx4_ib
    mlx4_en
    ib_uverbs
    rdma_ucm

חבילות נדרשות לניסוי:

-   sockperf

-   vma

-   cpufrequtils

-   perftest

-   libibverbs

-   librdmacm

-   librdmacm-dev

-   cmake

### Huge pages

Set up huge pages for VMA, by setting the file `/etc/sysctl.d/99-hugepages.conf` to:

    kernel.shmmax = 1000000000
    vm.nr_hugepages = 800

