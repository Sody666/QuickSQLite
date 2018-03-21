//
//  UIScrollView+constraint.h
//  QuickVFL
//
//  Created by Sou Dai on 16/9/21.
//  Copyright © 2016 Sou Dai. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef enum : NSUInteger {
    QScrollOrientationVertical = 0,
    QScrollOrientationHorizontal,
} QScrollOrientation;

@interface UIScrollView(constraint)
/**
 *  Prepare the content view for scroll view.
 *
 *  @param orientation the orientation for future use
 *
 *  @return prepared content view
 */
-(UIView*)q_prepareContentViewForOrientation:(QScrollOrientation)orientation;

/**
 *  Refresh the content view.
 *  Make sure the orientation is set with q_prepareContentViewForOrientation:
 */
-(void)q_refreshContentView;


@property (nonatomic, weak, readonly) UIView* q_contentView;
@end
