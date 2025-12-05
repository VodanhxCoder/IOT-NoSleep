import React, { useState, useEffect } from 'react';
import { Users, UserX, UserCheck, Trash2, Shield, ShieldOff, Activity, UserPlus, X } from 'lucide-react';
import userService from '../services/userService';
import { useAuth } from '../context/AuthContext';

const AdminDashboard = () => {
  const [users, setUsers] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const [showCreateModal, setShowCreateModal] = useState(false);
  const [newUser, setNewUser] = useState({ username: '', email: '', password: '', role: 'user' });
  const { user: currentUser } = useAuth();

  useEffect(() => {
    fetchUsers();
  }, []);

  const fetchUsers = async () => {
    try {
      setLoading(true);
      const response = await userService.getUsers();
      if (response.success) {
        setUsers(response.data);
      }
    } catch (err) {
      setError('Failed to fetch users');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  const handleCreateUser = async (e) => {
    e.preventDefault();
    try {
      await userService.createUser(newUser);
      setShowCreateModal(false);
      setNewUser({ username: '', email: '', password: '', role: 'user' });
      fetchUsers();
      alert('User created successfully');
    } catch (err) {
      console.error('Failed to create user:', err);
      alert(err.response?.data?.message || 'Failed to create user');
    }
  };

  const handleBanToggle = async (user) => {
    if (user._id === currentUser.id) return;
    
    if (user.username === 'MinhKhue123') {
      alert('Cannot ban the device account (MinhKhue123)');
      return;
    }

    try {
      if (user.isBanned) {
        await userService.unbanUser(user._id);
      } else {
        if (window.confirm(`Are you sure you want to ban ${user.username}?`)) {
          await userService.banUser(user._id);
        } else {
          return;
        }
      }
      fetchUsers();
    } catch (err) {
      console.error('Failed to toggle ban status:', err);
      alert('Failed to update user status');
    }
  };

  const handleDelete = async (user) => {
    if (user._id === currentUser.id) return;

    if (user.username === 'MinhKhue123') {
      alert('Cannot delete the device account (MinhKhue123)');
      return;
    }

    if (window.confirm(`Are you sure you want to DELETE user ${user.username}? This action cannot be undone.`)) {
      try {
        await userService.deleteUser(user._id);
        fetchUsers();
      } catch (err) {
        console.error('Failed to delete user:', err);
        alert('Failed to delete user');
      }
    }
  };

  const isOnline = (lastActive) => {
    if (!lastActive) return false;
    const fiveMinutesAgo = new Date(Date.now() - 5 * 60 * 1000);
    return new Date(lastActive) > fiveMinutesAgo;
  };

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center bg-gray-50 dark:bg-gray-900">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-primary-600"></div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900 py-8">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="mb-8">
          <h1 className="text-3xl font-bold text-gray-900 dark:text-white flex items-center gap-3">
            <Shield className="w-8 h-8 text-primary-600" />
            Admin Dashboard
          </h1>
          <p className="text-gray-600 dark:text-gray-400 mt-2">Manage users and system access</p>
        </div>

        {error && (
          <div className="bg-red-100 border border-red-400 text-red-700 px-4 py-3 rounded relative mb-6">
            {error}
          </div>
        )}

        <div className="bg-white dark:bg-gray-800 shadow overflow-hidden sm:rounded-lg">
          <div className="px-4 py-5 sm:px-6 border-b border-gray-200 dark:border-gray-700 flex justify-between items-center">
            <h3 className="text-lg leading-6 font-medium text-gray-900 dark:text-white flex items-center gap-2">
              <Users className="w-5 h-5" />
              User Management
            </h3>
            <div className="flex items-center gap-3">
              <span className="bg-blue-100 text-blue-800 text-xs font-medium px-2.5 py-0.5 rounded dark:bg-blue-900 dark:text-blue-300">
                Total Users: {users.length}
              </span>
              <button
                onClick={() => setShowCreateModal(true)}
                className="btn-primary flex items-center gap-2 text-sm py-1.5"
              >
                <UserPlus className="w-4 h-4" />
                Add User
              </button>
            </div>
          </div>
          
          <div className="overflow-x-auto">
            <table className="min-w-full divide-y divide-gray-200 dark:divide-gray-700">
              <thead className="bg-gray-50 dark:bg-gray-700">
                <tr>
                  <th scope="col" className="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">
                    User
                  </th>
                  <th scope="col" className="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">
                    Role
                  </th>
                  <th scope="col" className="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">
                    Status
                  </th>
                  <th scope="col" className="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">
                    Last Active
                  </th>
                  <th scope="col" className="px-6 py-3 text-right text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">
                    Actions
                  </th>
                </tr>
              </thead>
              <tbody className="bg-white dark:bg-gray-800 divide-y divide-gray-200 dark:divide-gray-700">
                {users.map((user) => (
                  <tr key={user._id} className={user.isBanned ? 'bg-red-50 dark:bg-red-900/10' : ''}>
                    <td className="px-6 py-4 whitespace-nowrap">
                      <div className="flex items-center">
                        <div className="flex-shrink-0 h-10 w-10 rounded-full bg-gray-200 dark:bg-gray-600 flex items-center justify-center">
                          <span className="text-xl font-medium text-gray-600 dark:text-gray-300">
                            {user.username.charAt(0).toUpperCase()}
                          </span>
                        </div>
                        <div className="ml-4">
                          <div className="text-sm font-medium text-gray-900 dark:text-white">
                            {user.username}
                            {user._id === currentUser.id && (
                              <span className="ml-2 text-xs text-gray-500">(You)</span>
                            )}
                          </div>
                          <div className="text-sm text-gray-500 dark:text-gray-400">
                            {user.email}
                          </div>
                        </div>
                      </div>
                    </td>
                    <td className="px-6 py-4 whitespace-nowrap">
                      <span className={`px-2 inline-flex text-xs leading-5 font-semibold rounded-full ${
                        user.role === 'admin' 
                          ? 'bg-purple-100 text-purple-800 dark:bg-purple-900 dark:text-purple-200' 
                          : user.role === 'manager'
                            ? 'bg-blue-100 text-blue-800 dark:bg-blue-900 dark:text-blue-200'
                            : 'bg-gray-100 text-gray-800 dark:bg-gray-700 dark:text-gray-300'
                      }`}>
                        {user.role}
                      </span>
                    </td>
                    <td className="px-6 py-4 whitespace-nowrap">
                      <div className="flex flex-col gap-1">
                        {user.isBanned ? (
                          <span className="px-2 inline-flex text-xs leading-5 font-semibold rounded-full bg-red-100 text-red-800 dark:bg-red-900 dark:text-red-200 w-fit">
                            Banned
                          </span>
                        ) : (
                          <span className="px-2 inline-flex text-xs leading-5 font-semibold rounded-full bg-green-100 text-green-800 dark:bg-green-900 dark:text-green-200 w-fit">
                            Active
                          </span>
                        )}
                        
                        {isOnline(user.lastActive) && (
                          <span className="flex items-center gap-1 text-xs text-green-600 dark:text-green-400">
                            <Activity className="w-3 h-3" /> Online
                          </span>
                        )}
                      </div>
                    </td>
                    <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500 dark:text-gray-400">
                      {user.lastActive ? new Date(user.lastActive).toLocaleString() : 'Never'}
                    </td>
                    <td className="px-6 py-4 whitespace-nowrap text-right text-sm font-medium">
                      {user._id !== currentUser.id && (
                        <div className="flex justify-end gap-3">
                          {/* Hide actions if Manager tries to act on Admin */}
                          {!(currentUser.role === 'manager' && user.role === 'admin') && (
                            <>
                              <button
                                onClick={() => handleBanToggle(user)}
                                className={`text-sm flex items-center gap-1 ${
                                  user.isBanned 
                                    ? 'text-green-600 hover:text-green-900 dark:hover:text-green-400' 
                                    : 'text-orange-600 hover:text-orange-900 dark:hover:text-orange-400'
                                }`}
                                title={user.isBanned ? "Unban User" : "Ban User"}
                              >
                                {user.isBanned ? <UserCheck className="w-4 h-4" /> : <UserX className="w-4 h-4" />}
                              </button>
                              
                              <button
                                onClick={() => handleDelete(user)}
                                className="text-red-600 hover:text-red-900 dark:hover:text-red-400 flex items-center gap-1"
                                title="Delete User"
                              >
                                <Trash2 className="w-4 h-4" />
                              </button>
                            </>
                          )}
                        </div>
                      )}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>

      {/* Create User Modal */}
      {showCreateModal && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center p-4 z-50">
          <div className="bg-white dark:bg-gray-800 rounded-lg max-w-md w-full p-6">
            <div className="flex justify-between items-center mb-4">
              <h3 className="text-lg font-bold text-gray-900 dark:text-white">Create New User</h3>
              <button onClick={() => setShowCreateModal(false)} className="text-gray-500 hover:text-gray-700 dark:hover:text-gray-300">
                <X className="w-5 h-5" />
              </button>
            </div>
            
            <form onSubmit={handleCreateUser} className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Username</label>
                <input
                  type="text"
                  required
                  value={newUser.username}
                  onChange={(e) => setNewUser({...newUser, username: e.target.value})}
                  className="input"
                  placeholder="Enter username"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Email</label>
                <input
                  type="email"
                  required
                  value={newUser.email}
                  onChange={(e) => setNewUser({...newUser, email: e.target.value})}
                  className="input"
                  placeholder="Enter email"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Password</label>
                <input
                  type="password"
                  required
                  value={newUser.password}
                  onChange={(e) => setNewUser({...newUser, password: e.target.value})}
                  className="input"
                  placeholder="Enter password"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Role</label>
                <select
                  value={newUser.role}
                  onChange={(e) => setNewUser({...newUser, role: e.target.value})}
                  className="input"
                >
                  <option value="user">User</option>
                  <option value="manager">Manager</option>
                </select>
              </div>

              <div className="flex justify-end gap-3 mt-6">
                <button
                  type="button"
                  onClick={() => setShowCreateModal(false)}
                  className="btn-secondary"
                >
                  Cancel
                </button>
                <button
                  type="submit"
                  className="btn-primary"
                >
                  Create User
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
};

export default AdminDashboard;
