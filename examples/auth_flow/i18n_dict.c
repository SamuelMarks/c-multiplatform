/**
 * @file i18n_dict.c
 * @brief Internationalization translation dictionary implementation.
 */

/* clang-format off */
#include "i18n_dict.h"
#include <string.h>
/* clang-format on */

/**
 * @brief Structure representing a single translation key and its localized
 * strings.
 */
struct i18n_entry {
  const char *key;   /**< The translation key */
  const char *en_US; /**< US English string */
  const char *es_ES; /**< Spain Spanish string */
  const char *fr_FR; /**< France French string */
};

/** @brief The hardcoded translation dictionary */
static const struct i18n_entry DICTIONARY[] = {
    {"app_title", "Auth Flow Example", "Ejemplo de Autenticación",
     "Exemple d'Authentification"},
    {"btn_theme_toggle", "Toggle dark mode", "Alternar modo oscuro",
     "Basculer en mode sombre"},
    {"btn_lang_toggle", "Switch language", "Cambiar idioma",
     "Changer de langue"},
    {"tab_login", "Login", "Iniciar sesión", "Connexion"},
    {"tab_signup", "Signup", "Registrarse", "S'inscrire"},
    {"lbl_username", "Username", "Nombre de usuario", "Nom d'utilisateur"},
    {"lbl_password", "Password", "Contraseña", "Mot de passe"},
    {"btn_submit_login", "Log In", "Entrar", "Se connecter"},
    {"btn_submit_signup", "Sign Up", "Crear cuenta", "Créer un compte"},
    {"msg_welcome_secret", "Welcome, %s!", "¡Bienvenido, %s!",
     "Bienvenue, %s !"},
    {"btn_logout", "Logout", "Cerrar sesión", "Se déconnecter"},
    {"err_invalid_credentials", "Invalid username or password.",
     "Usuario o contraseña inválidos.",
     "Nom d'utilisateur ou mot de passe invalide."},
    {"err_user_exists", "Username already exists.", "El usuario ya existe.",
     "Ce nom d'utilisateur existe déjà."}};

ui_error_t get_translated_string(const char *locale, const char *key,
                                 const char **out_str) {
  size_t i;
  size_t count = sizeof(DICTIONARY) / sizeof(DICTIONARY[0]);

  if (!out_str) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!locale)
    locale = "en-US";

  for (i = 0; i < count; i++) {
    if (strcmp(DICTIONARY[i].key, key) == 0) {
      if (strcmp(locale, "es-ES") == 0) {
        *out_str = DICTIONARY[i].es_ES;
        return UI_ERROR_NONE;
      }
      if (strcmp(locale, "fr-FR") == 0) {
        *out_str = DICTIONARY[i].fr_FR;
        return UI_ERROR_NONE;
      }
      *out_str = DICTIONARY[i].en_US;
      return UI_ERROR_NONE;
    }
  }
  *out_str = key; /* Fallback to key if not found */
  return UI_ERROR_NOT_FOUND;
}
