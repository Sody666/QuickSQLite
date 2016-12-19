# Quick SQLite
Quick SQLite项目的目标是简化iOS环境中sqlite数据库的使用，免除各种难看的hardcoding等。
## 特点
- 支持insert、update、delete、query操作
- 支持存储过程
- 支持bundle数据库自动更新替换
## 例子
```objective-c
// 插入数据
{
	NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
	[contentValues addObject:[QDBValue instanceForObject:@(21) withKey:kColumnAge]];
	[contentValues addObject:[QDBValue instanceForObject:@(175) withKey:kColumnHeight]];
	long long recordId = [helper insert:kTableName contentValues:contentValues];
}
// 更新数据
{
	NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"Bob" withKey:kColumnName]];
    [contentValues addObject:[QDBValue instanceForObject:@(19) withKey:kColumnAge]];
        
    NSString* where = [NSString stringWithFormat:@"%@==1", kColumnId];
    long affactedCount = [helper update:kTableName contentValues:contentValues where:where];
}
// 查询数据
{
	NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    NSArray* rows = [helper query:kTableName columns:@[kColumnName] where:where];
	// 将返回 [{"name":"Bob"}]
}
// 存储过程
{
	[helper beginTransactionWithError:nil];
    // insert a record
    contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"Bob" withKey:kColumnName]];
    recordId = [helper insert:kTableName contentValues:contentValues];
	// 这里还可以做千千万万的插入、更新、删除操作
    [helper endTransaction];
}

```
