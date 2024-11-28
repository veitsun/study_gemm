# get_file_list

这个函数的作用是遍历指定目录中的所有文件，并对每个文件执行一些操作：

1. 打开指定的目录路径
2. 逐个读取目录中的文件/文件夹
3. 跳过以"."开头的隐藏文件和特殊目录（如"."和".."）
4. 对于每个非隐藏的文件：
   - 构建文件的相对路径
   - 将相对路径转换为绝对路径
   - 检查是否为普通文件（不是目录、链接等）
   - 如果是普通文件，调用`base.count_lines()`方法处理文件
5. 将处理结果写入到指定的文件列表中

具体步骤解析：
- `opendir(path)` 打开指定目录
- `readdir(dp)` 逐个读取目录中的条目
- `realpath()` 将相对路径转换为绝对路径
- `stat()` 获取文件状态信息
- `S_ISREG()` 检查是否为普通文件
- `base.count_lines()` 对文件进行行数统计或其他处理
- 处理结果写入 `filelist` 文件中

简单来说，这是一个遍历目录并处理目录中所有普通文件的函数，最终生成一个文件列表。


## count_lines

这个函数的作用是统计文件的行数，并按特定间隔记录文件的行信息和文件偏移量。详细解释如下：

1. 打开指定的文件
2. 初始化行数计数器 `lines`
3. 逐行读取文件内容
4. 按照固定的行间隔（`LINE_GROUPS`）记录文件信息到 `filelist`
5. 具体记录的信息包括：
   - 文件名
   - 当前行数
   - 文件偏移量（文件中的读取位置）

主要功能：
- 统计文件总行数
- 创建文件的"分块"信息
- 记录每个分块的起始行和文件偏移量

例如，如果 `LINE_GROUPS` 是 10，那么每隔 10 行就会在 `filelist` 文件中写入一条记录，记录当前文件名、行数和文件位置。

这种方法可以用于：
- 文件大小预估
- 文件分块处理
- 并行文件处理的索引信息

返回值是文件的总行数。

示例：
```
filename1.txt   0   0
filename1.txt   10  300
filename1.txt   20  600
...
```


# delete_directory

这个函数的作用是递归删除指定目录及其所有内容（包括子目录和文件）。具体步骤如下：

1. 打开指定路径的目录
2. 遍历目录中的所有条目
3. 跳过特殊目录"."和".."
4. 对于每个目录项：
   - 构建完整的文件/目录路径
   - 如果是子目录，递归调用`delete_directory()`删除子目录
   - 如果是文件，使用`remove()`删除文件
   - 记录删除操作的结果
5. 最后删除当前目录本身

关键特点：
- 递归删除：能够删除多层嵌套的目录结构
- 安全处理：跳过"."和".."避免意外删除
- 错误处理：对删除失败的文件打印错误信息

使用场景：
- 清理临时文件夹
- 完全删除一个目录及其所有内容
- 文件系统清理操作

注意事项：
- 使用时要非常小心，因为这是不可逆的删除操作
- 需要确保有足够的权限删除目录和文件

典型用法：
```cpp
Solver solver;
solver.delete_directory("/path/to/directory");
```

这个函数会删除指定路径下的所有文件和子目录，最后删除路径本身。

# sort_list

这个函数的作用是并行排序文件的各个部分，主要步骤如下：

1. 创建线程池（线程数由`THREAD_NUM`决定）
2. 打开文件列表（之前通过`count_lines`生成的列表）
3. 逐行读取文件列表，每一行包含：
   - 原始文件名
   - 起始行号
   - 文件偏移量
4. 对每个文件块：
   - 生成临时目标文件名（在目标目录下，以0开头）
   - 提交排序任务到线程池
   - 使用`sort_worker`函数作为工作线程
5. 等待所有排序任务完成
6. 关闭线程池

关键点：
- 使用线程池实现并行排序
- 每个文件块独立排序
- 生成的临时文件以0开头，便于后续合并
- 通过`future`机制等待所有任务完成

典型使用场景：
- 大文件分块排序
- 利用多线程提高排序性能
- 处理超大文件无法一次性加载到内存的情况

函数流程：
1. 初始化线程池
2. 读取文件列表
3. 为每个文件块创建排序任务
4. 收集并等待所有任务结果
5. 关闭线程池


## sort_worker
这个函数是一个排序工作线程，主要功能是对文件的特定部分进行排序并写入新文件。具体步骤如下：

1. 接收参数：
   - `name`：源文件名
   - `start_line`：起始行号
   - `offset`：文件偏移量
   - `lines`：要处理的行数
   - `dst_name`：目标文件名

2. 文件读取：
   - 打开源文件
   - 移动文件指针到指定偏移量
   - 读取指定数量的行
   - 将读取的行转换为整数存储在`data_buffer`中

3. 排序：
   - 使用`merge_sort`对`data_buffer`中的数据进行排序
   - 排序范围是0到实际读取的行数

4. 写入结果：
   - 打开目标文件
   - 将排序后的数据写入目标文件
   - 每行写入一个整数

5. 清理：
   - 关闭文件
   - 释放动态分配的内存（`name`和`dst_name`）

关键特点：
- 处理文件的部分内容
- 就地排序
- 支持并行处理文件块
- 错误处理（文件打开失败）

典型使用场景：
- 大文件分块排序
- 多线程文件处理
- 外部排序算法的一部分

主要目的是将大文件分成小块，并行排序，为后续合并做准备。


# merge_orderd_files

这个函数的作用是实现多文件的归并排序，具体步骤如下：

1. 终止条件检查：
   - 如果指定前缀的文件数量小于等于1，说明已经完成最终合并
   - 返回当前前缀，表示排序结束

2. 归并排序过程：
   - 打开指定目录
   - 创建线程池
   - 遍历目录中以特定前缀开头的文件

3. 文件配对和合并：
   - 两两配对待合并的文件
   - 对每对文件创建合并任务
   - 使用`merge_two_file`函数合并文件
   - 生成新的带增加前缀的输出文件

4. 特殊情况处理：
   - 如果最后剩单数文件，与NULL合并
   - 等待所有合并任务完成

5. 递归调用：
   - 完成当前前缀的合并后
   - 递归调用自身，前缀加1
   - 继续合并下一轮文件

关键特点：
- 多线程并行归并
- 递归处理文件合并
- 逐步减少文件数量
- 最终生成单个排序文件

典型使用场景：
- 大规模外部排序
- 处理超大文件集合的排序
- 分治排序算法的实现

算法流程：
1. 将文件两两配对
2. 合并每对文件
3. 生成新的带增加前缀的文件
4. 递归处理，直到只剩一个文件


## count_prefix

这个函数的作用是统计指定目录中以特定前缀开头的文件数量。具体步骤如下：

1. 接收两个参数：
   - `prefix`：要统计的文件前缀数字
   - `dir_path`：要搜索的目录路径

2. 文件统计过程：
   - 将前缀数字转换为字符串
   - 打开指定目录
   - 遍历目录中的所有文件
   - 检查每个文件名是否以指定前缀开头
   - 统计匹配的文件数量

3. 错误处理：
   - 如果目录无法打开，打印错误信息

4. 返回值：
   - 返回以指定前缀开头的文件数量

关键特点：
- 简单的文件名前缀匹配
- 目录遍历
- 文件计数

典型使用场景：
- 在大规模文件排序过程中，检查当前归并阶段的文件数量
- 确定是否需要继续合并文件
- 作为归并排序算法的辅助函数

在`merge_orderd_files`函数中，这个函数用于判断是否已完成最终的文件合并。当只剩一个带特定前缀的文件时，意味着排序过程已经完成。


## merge_two_file

这个函数的作用是合并两个已排序的文件，生成一个新的有序文件。具体步骤如下：

1. 文件打开和错误处理：
   - 打开两个输入文件和一个输出文件
   - 检查文件是否成功打开
   - 处理文件打开失败的各种情况

2. 特殊情况处理：
   - 如果其中一个文件为空（NULL），直接复制另一个文件的内容
   - 确保不会因为空文件导致程序崩溃

3. 归并排序合并：
   - 同时读取两个文件的当前行
   - 比较两行的整数值
   - 将较小的值写入输出文件
   - 移动对应文件的指针

4. 归并过程细节：
   - 使用`atoi()`将字符串转换为整数进行比较
   - 持续合并，直到两个文件都读完
   - 确保输出文件是有序的

5. 资源清理：
   - 关闭所有文件
   - 释放动态分配的内存（文件名字符串）

关键特点：
- 处理两个已排序文件的合并
- 支持不等长文件的合并
- 处理文件为空的边界情况
- 线性时间复杂度的归并

典型使用场景：
- 外部排序算法的归并步骤
- 合并已排序的文件块
- 多路归并排序的实现

算法流程：
1. 读取两个文件的当前行
2. 比较行的整数值
3. 写入较小值的行
4. 移动对应文件指针
5. 重复直到两个文件都读完

这个函数是大规模文件排序算法中的关键组成部分，负责将已排序的小文件块逐步合并成更大的有序文件。


# copy_prefix_file

这个函数的作用是在指定的目录中查找以特定前缀开头的文件，并将该文件的内容复制到目标文件中。下面是详细解释：

函数参数：
- `path`：要搜索的目录路径
- `prefix`：文件名必须以此数字前缀开头
- `dst_name`：目标文件名（复制内容的目标文件）

函数执行步骤：
1. 首先进行参数有效性检查，确保传入的路径、前缀和目标文件名有效。

2. 打开指定目录（使用`opendir()`）。

3. 打开目标文件（使用`fopen()`），准备写入。

4. 遍历目录中的所有文件：
   - 将前缀数字转换为字符串
   - 检查每个文件名是否以该前缀开头
   - 如果找到匹配的文件，则：
     * 构建完整的源文件路径
     * 打开源文件
     * 逐行读取源文件内容
     * 将内容写入目标文件
     * 关闭源文件
     * 结束搜索（只处理第一个匹配的文件）

5. 最后关闭目录和目标文件。

注意事项：
- 函数只处理第一个匹配前缀的文件
- 如果目录打开失败或目标文件创建失败，会打印错误信息并返回
- 使用了固定大小的缓冲区（`LINE_BUF_SIZE`和`PATH_BUF_SIZE`）

使用示例：
```c
// 在 "/home/user/documents" 目录中
// 查找以 "123" 开头的文件
// 将其内容复制到 "output.txt"
copy_prefix_file("/home/user/documents", 123, "output.txt");
```