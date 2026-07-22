Import("env")

# Очистка проекта
env.Execute("pio run --target clean")

# Сборка проекта
env.Execute("pio run")