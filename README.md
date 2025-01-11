# 📌 Squared

Squared is a WIP Private Project that will be solely developed by myself. The aim is to create a project that helps me gain familiarity / comfort with C++, RayLib and Make. Make is primarily used to help with ease of compilation on Linux. 

The game is aimed to be an endless runner where the player can wrap around the game window. The idea is to kill enemies, score points and survive as long as possible.

# 📚 Libraries
- [RayLib](https://github.com/raysan5/raylib)
- [Make](https://developers.make.com/api-documentation/make-api-documentation)

# 🔨 Build | Linux Only.
- Open `Terminal`.
- Navigate to the folder.
- `make` will build & run the project.

# ✅ Features
- Game Timer.
- Pause / Resume Functionality.
- Basic Enemy Movement.
- Player Health / Lives System.
- Basic Scoring System.

# 🔧 TODO
- Seperate into Header / CPP Files.
  - `static inline` will be corrected with this. I need to read more about the benefits.
  - `static inline` was fixed with Pointers & References instead. This will make seperation easier.
- Enemies Attack Player.
  - Enemies will deal collision damage to the player upon impact.
- Player Health Bar.
- Enemy Waves & Timer.
  - Implemented in a basic manner. Enemies will spawn in waves based on game timer. 
  - Number of Enemies spawned are based on `std::map` of `gameLevel` -> `numEnemies`.
- Different Projectiles.
- Wave Duration.
  - Indicate how long a wave has taken to be killed.
  - Issue: As enemy count increases, this duration will become more relevant but harder to track as new waves will be spawning.
- Enemies Killed / Waves Killed Counter.
  - Interesting metric for the user.

# 🖌️ Assets
- Assets are created by myself, using Aseprite. This is still very much a learning experience.
<table align="center">
  <tr>
    <th style="text-align:center;">Asset Name</th>
    <th style="text-align:center;">Image</th>
  </tr>
  <tr>
    <td style="text-align:center;">Player Character</td>
    <td style="text-align:center;"><img src="/Resources/Assets/Player.png" alt="Player Character" width="32"></td>
  </tr>
  <tr>
    <td style="text-align:center;">Enemy Character</td>
    <td style="text-align:center;"><img src="/Resources/Assets/Enemy.png" alt="Enemy Character" width="32"></td>
  </tr>
  <tr>
    <td style="text-align:center;">Projectile</td>
    <td style="text-align:center;"><img src="/Resources/Assets/Projectile.png" alt="Projectile" width="32"></td>
  </tr>
  <tr>
    <td style="text-align:center;">Pickups: Health</td>
    <td style="text-align:center;"><img src="/Resources/Assets/Health.png" alt="Pickup: Health" width="32"></td>
  </tr>
  <tr>
    <td style="text-align:center;">Pickups: Life</td>
    <td style="text-align:center;"><img src="/Resources/Assets/Life.png" alt="Pickup: Life" width="32"></td>
  </tr>
  <tr>
    <td style="text-align:center;">Pickups: PowerUp</td>
    <td style="text-align:center;"><img src="/Resources/Assets/PowerUp.png" alt="Pickup: PowerUp" width="32"></td>
  </tr>
</table>