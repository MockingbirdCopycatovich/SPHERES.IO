# 🎮 SPHERES.IO

**SPHERES.IO** is a 3D game, developed in **C++** using the **TL-Engine**.  
The player controls a sphere, collects cubes to grow in size, activates special abilities, and competes against an AI-controlled enemy sphere.

This project was created as part of **CO1301 Games Concepts – Assessment 1**.

---

## 📌 Student Information

- **Student Name:** Vladislav Vasilev  
- **Student ID:** G21303193  
- **Module:** CO1301 Games Concepts  
- **Assessment:** Assessment 1  

---

## 🎥 Gameplay Video

▶️ **Gameplay video:**  
*(insert link here)*

Example:


The video demonstrates the core gameplay mechanics, scoring system, Hyper Mode, enemy AI behaviour, game states, and win/lose conditions.

---

## 🕹️ Gameplay Overview

The player controls a **Player Sphere** on an island surrounded by water.  
An **Enemy Sphere** (AI) is also present and competes with the player by collecting cubes and growing in size.

### 🔹 Core Mechanics

#### 🟢 Cube Collection
- There are **12 regular cubes** on the map at all times.
- Collecting a cube:
  - Grants **+10 points**
  - Respawns the cube at a random valid position
- Every **40 points**:
  - The sphere increases in size
  - The collision radius increases accordingly

#### ⚡ Hyper Cube & Hyper Mode
- A special **Hyper Cube** spawns on the map.
- When collected, it activates **Hyper Mode** for **5 seconds**:
  - Increased movement speed
  - Attraction of nearby cubes
  - Sphere texture changes
- Hyper Mode can be activated by **both the player and the enemy**

#### 🔴 Enemy Sphere (AI)
- The enemy automatically:
  - Searches for the closest cube
  - Prioritises the Hyper Cube when it is active
  - Collects cubes and grows in size
- After the game ends, the enemy continues moving in an idle state

#### ⚔️ Sphere Collisions
- If the score difference is **small**, spheres bounce off each other
- If the score difference is **greater than 40**:
  - The stronger sphere consumes the weaker one
  - The game immediately ends

---

## 🏆 Win & Lose Conditions

### ✅ Player Wins
- The player reaches **120 points**
- Or defeats the enemy during a sphere collision

### ❌ Game Over
- The enemy reaches **120 points**
- The player falls into the water (leaves the island boundaries)
- The player loses a collision against a stronger enemy

---

## 🎮 Controls

### Player
- **W / S** — Move forward / backward  
- **A / D** — Rotate left / right  
- **P** — Pause / Unpause  
- **ESC** — Quit the game  

### Camera
- **1** — Top-down camera view  
- **2** — Isometric camera view  
- **Arrow Keys** — Move camera (top-down view only)

---

## 🛠️ Technologies Used

- **Language:** C++  
- **Engine:** TL-Engine (TLX)  
- **Platform:** Windows  

---

## ▶️ How to Run the Project

1. **Download and install TL-Engine**  
   https://www.tlengine.com

2. **Create a new C++ project in TL-Engine**
   - Launch TL-Engine
   - Create a new C++ project

3. **Copy the source code**
   - Open the `test.cpp` file in the newly created project
   - Replace its contents with the code from this repository

4. **Add game resources**
   - Copy the **`Resources`** folder from this repository
   - Paste it into the TL-Engine project directory  
     *(on the same level as `test.cpp`)*

5. **Build and run**
   - Compile the project
   - Run the game using TL-Engine

---

## 📁 Project Structure

SPHERES.IO
│
├── SpheresIO.cpp
├── Resources/
│   ├── island.x
│   ├── water.x
│   └── etc.
├── LICENCE
├── .gitignore
└── README.md

## 📌 Notes

- This project is designed to run **only with TL-Engine**
- The game will **not work correctly without the `Resources` folder**
- **Frame-rate independent movement** is used throughout the game