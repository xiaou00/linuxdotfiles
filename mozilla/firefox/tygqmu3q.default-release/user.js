// enable userChrome.css
user_pref("toolkit.legacyUserProfileCustomizations.stylesheets", true);
user_pref("browser.compactmode.show", true);
user_pref("browser.uidensity", 1);
user_pref("browser.display.background_color", "#151515");
user_pref("browser.display.background_color.dark", "#151515");
// Keep light-background sites from inheriting the system theme's white text.
user_pref("layout.css.prefers-color-scheme.content-override", 1);
