/* clang-format off */
#include "cmp_system_theme.h"

#if defined(__APPLE__) && defined(__MACH__)

#include <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#else
#import <UIKit/UIKit.h>
#endif

int cmp_system_theme_init(void) {
  return 0;
}

int cmp_system_theme_shutdown(void) {
  return 0;
}

int cmp_system_theme_is_dark(int *out_is_dark) {
  if (!out_is_dark) return -1;

#if TARGET_OS_OSX
  NSAppearance *appearance = [NSApp effectiveAppearance];
  if ([appearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameDarkAqua, NSAppearanceNameAqua]] == NSAppearanceNameDarkAqua) {
      *out_is_dark = 1;
  } else {
      *out_is_dark = 0;
  }
#else
  UIUserInterfaceStyle style = UITraitCollection.currentTraitCollection.userInterfaceStyle;
  if (style == UIUserInterfaceStyleDark) {
      *out_is_dark = 1;
  } else {
      *out_is_dark = 0;
  }
#endif

  return 0;
}

int cmp_system_theme_is_high_contrast(int *out_is_high_contrast) {
  if (!out_is_high_contrast) return -1;
#if TARGET_OS_OSX
  *out_is_high_contrast = NSWorkspace.sharedWorkspace.accessibilityDisplayShouldIncreaseContrast ? 1 : 0;
#else
  *out_is_high_contrast = UIAccessibilityDarkerSystemColorsEnabled() ? 1 : 0;
#endif
  return 0;
}
#endif /* __APPLE__ */
/* clang-format on */