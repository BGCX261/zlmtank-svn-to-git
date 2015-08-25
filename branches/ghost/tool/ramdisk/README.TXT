1. 驱动程序的配置信息存放在下面注册表中：
   HKLM\System\CurrentControlSet\Services\RRamdisk\Parameters\
   为什么要用RRamdisk这个名字，因为XP SP1开始内置了一个ramdisk程序，
   主要用于XP Embed内存运行的。

2. 参数: DiskSizeM, DiskSizeK
   以M和K为单位的虚拟盘大小，K单位优先。

3. 参数: MediaType
    虚拟盘模拟的磁盘类型：
       1 -- 内存盘
       2 -- 固定介质 (缺省)
       3 -- 移动介质
       4 -- 软驱，和移动盘区别不大

4. 参数: DriveLetter
   所用盘符，缺省为 R:

5. 参数: SectorsPerCluster 
   希望的簇大小, 零由驱动程序决定:
       <= 2M		FAT12, 簇=512
       <= 32M		FAT16, 簇=512
       <= 64M		FAT16, 簇=1k
       <= 128M		FAT16, 簇=2k
       <= 256M		FAT16, 簇=4k
       <= 512M		FAT32, 簇=2k
       <= 16G		FAT32, 簇=4k
       ......
   如果你选择的簇大小比缺省的小，分区会被格式化成FAT32格式。

6. 参数: Image
   由rdutil程序管理，主要用以自动加载NTFS映像. 例如:
   1) 加载虚拟盘，不要做任何操作
   2) 格式化为NTFS
         FORMAT /FS:NTFS /Q /V:RamDisk /A:512 R:
   3) 把NTFS的日志大小设到最小（2M）
         CHKDSK /L:2048 R:
   4) 预设访问权限（例子是只允许管理员们使用虚拟盘）
         CACLS R:\ /G: BUILTIN\Adminstrators:F
   5) 创建一些目录结构，如临时目录
         MKDIR R:\TEMP
   6) 压缩虚拟盘，并存到这个注册表中
         rdutil R: registry

   注释:
   1) 你应该尽量不要往虚拟盘上放太多东西，压缩后的映像文件不能超过64K
   2) 当你改变虚拟盘大小时必须重新创建映像文件
   3) 当映像文件大小和虚拟盘大小不符时，虚拟盘将不再自动格式化。
      如果要驱动自动格式化，必须删除这个注册表项。

   rdutil 程序使用示例:
   1) 备份注册表里的映像文件
         rdutil save filename
   2) 恢复注册表里的映像文件
         rdutil load filename
   3) 测试压缩映像文件大小
         rdutil R:
	 rdutil registry
	 rdutil unpacked_file
      第二条命令压缩注册表里设置的缺省盘符
      第三条命令压缩被解开的映像文件
   4) 压缩盘符并存入注册表
         rdutil R: registry
   5) 压缩盘符但保存到文件
	 rdutil R: packed_file
   6) 把映像文件解开
         rdutil unpack packed_file unpacked_file
   7) 创建一个NTFS符号连接直接指向虚拟盘
         rdutil link temp \

7. 重新格式化虚拟盘
   除了内存介质类型外，其它的盘符都可以重新格式化。如：
   固定介质
       FORMAT /FS:NTFS /FORCE /Q /V:RamDisk /A:512 R:
   移动介质
       ECHO Y | FORMAT /FS:NTFS /FORCE /Q /V:RamDisk /A:512 R:
   你可以在开机脚本里对虚拟盘重新格式化。但不推荐再次格式化为FAT分区，
   因为驱动程序格式化的分区比通用格式化程序效率更高。如果要把虚拟盘做
   成映像文件作它用（特别是软驱），推荐重新格书化为标准格式。

8. 页面交换文件支持
   如果你使用固定介质类型，并且不重新格式化，你可以用系统管理直接把
   页面文件加到虚拟盘上。 如果你使用其它介质类型，或者需要重新格式化，
   请使用addswap程序创建页面文件。 如：
       addswap r:\pagefile.sys 16 32
   创建一个页面文件，最小16M，最大32M

9. TEMP 临时目录
   驱动程序格式化时已经创建好了TEMP目录。需要的话要把TEMP和TMP环境变量
   设到子目录下，不能设置到根目录。

10. NTFS 符号连接到虚拟盘
    当使用内存盘介质类型时，不能从其它NTFS分区创建符号连接到虚拟盘。
    固定和移动介质都没有问题。

11. Connectix VirtualPC 兼容性
    如果VPC无法使用物理RAW磁盘时，不要使用固定介质类型。

12. 无盘符工作方式
    把DriveLetter注册表值设为空，驱动将不创建任何盘符。注意不是删除
    DriveLetter，否则缺省创建R：
    无盘符方式下，用rdutil创建ram目录指向虚拟盘（只能在NTFS分区里）：
	   rdutil link C:\ramdisk
	   	连接 C:\ramdisk 到虚拟盘根目录
	   rdutil link C:\TEMP TEMP
	   	连接 C:\TEMP 到虚拟盘的 \TEMP 下
	   rmdir C:\ramdisk
	   	rmdir 可以直接删除连接点，不会影响连接目标
    不推荐使用无盘符模式，因为大多是防毒程序无法实时监控无盘符的分区