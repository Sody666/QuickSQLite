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

```
