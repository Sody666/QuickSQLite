# Quick SQLite
Quick SQLite项目的目标是简化iOS环境中sqlite数据库的使用，免除各种难看的hardcoding等。
## 特点
- 支持insert、update、delete、query操作
- 支持存储过程
- 支持bundle数据库自动更新替换
- 支持标准的加密数据库

## 例子
```objective-c
// 插入数据
{
	NSDictionary* values = @{
					kColumnAge:@(21),
					kColumnHeight:@(175),
				};
	long long recordId = [helper insert:kTableName values:values];
}
// 更新数据
{
	NSDictionary* values = @{
					kColumnName:@"Bob",
					kColumnAge:@(19),
				};
        
    NSString* where = [NSString stringWithFormat:@"%@==1", kColumnId];
    long affactedCount = [helper update:kTableName values:values where:where];
}
// 查询数据
{
	NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    NSArray* rows = [helper query:kTableName columns:@[kColumnName] where:where];
	// 将返回 [{"name":"Bob"}]
}
// 存储过程
// 如果要插入大量数据，用这种方法，带来的性能提升是非常惊艳的
{
	[helper beginTransactionWithError:nil];
    // insert a record
    NSDictionary* values = @{
					kColumnAge:@(21),
					kColumnHeight:@(175),
				};
    recordId = [helper insert:kTableName values:values];
	// 这里还可以做千千万万的插入、更新、删除操作
    [helper endTransaction];
}

// 打开加密数据库
{
	// 在真实的环境中，不能这样传递数据库的密钥
	// 也就是，不能让别人通过破解你的ipa获得你的数据库密钥
	// 最好做到每台设备上的密钥都不一样
	helper = [[QSQLiteOpenHelper alloc] initWithName:kDBName key:kEncryptedDBKey openDelegate:self];
}
```
## 安装
### 直接使用现成的Framework
你可以直接使用本repo中，Release文件夹中的发布出来的framework。framework分为两个版本：
1. 共享版本。此版本没有把SQLCipher编辑进来，因而文件也比较小。当你的app里有其他framework使用了SQLCipher的时候，推荐用这个版本。解压此文件夹，你会看到里面有libSQlcipher，这是编译framework同时产生的。你需要用的时候可以使用。
2.独立版本。此版本把SQLCipher编译进来了。
你只要把framework下载，然后添加到项目里就可以，不用设置任何东西（有些时候需要在Build Phrases->Link Binary With Libries里，设置本框架为Optional）。

### 直接下载使用源代码
把source下所有的源文件拷贝到你的项目里就可以。但你要处理好跟SQLCipher的关系。
