//
//  QLayoutManager.h
//  LibSourceUser
//
//  Created by 苏第 on 17/11/5.
//  Copyright © 2017年 Su Di. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
typedef enum : NSUInteger {
    // print all the information & quit app if any error.
    QLayoutModeVerbose,
    // quit app if any error.
    QLayoutModeQuiet,
    // ignore any error.
    QLayoutModePeaceful,
} QLayoutMode;

@class QLayoutResult;

@interface QLayoutManager : NSObject

+(QLayoutResult*) layoutForFilePath:(NSString*)filePath
                           entrance:(UIView*)entrance
                             holder:(id)holder;

+(QLayoutResult*) layoutForFileName:(NSString*)fileName
                           entrance:(UIView*)entrance
                             holder:(id)holder;

+(QLayoutResult*) layoutForContent:(NSString*)content
                          entrance:(UIView*)entrance
                            holder:(id)holder;

+(Class)parseClassName:(NSString*)className;

+(void)setupLayoutMode:(QLayoutMode)mode;
+(QLayoutMode)layoutMode;

+(void)printTimingData;
+(void)resetTimeData;
@end
