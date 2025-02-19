export interface UserPreferences {
  defaultSymbol: string;
  theme: 'light' | 'dark';
  notifications: boolean;
}

export interface User {
    id: string;
    username: string;
    email: string;
    role: 'user' | 'admin';
    preferences: UserPreferences;
  }
  
  export interface LoginCredentials {
    email: string;
    password: string;
  }
  
  export interface AuthResponse {
    user: User;
    token: string;
  }

  export interface RegisterCredentials {
    username: string;
    email: string;
    password: string;
    confirmPassword: string;
  }
  
  export interface UserProfile {
    id: string;
    username: string;
    email: string;
    role: 'user' | 'admin';
    joinDate: Date;
    preferences: {
      defaultSymbol: string;
      theme: 'light' | 'dark';
      notifications: boolean;
    }
  }