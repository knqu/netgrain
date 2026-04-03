import '../styling/Login.css';
import { useState, useEffect } from 'react';
import { useNavigate } from "react-router-dom";

export default function Login() {
    const navigate = useNavigate();
    const [email, setEmail] = useState<string>("");
    const [password, setPassword] = useState<string>("");

    useEffect(() => {
        const checkSession = async () => {
            try {
                const response = await fetch(
                    "/api/cookieCheck",
                    {
                        method: "GET",
                    }
                );
                if (response.status == 200) {
                    navigate("/home");
                }
            } catch (err) {
                console.log(err);
            }
        };
        checkSession()
    }, []);

    const handleLogin = async (e : React.FormEvent<HTMLFormElement>) => {
        e.preventDefault();
        try {
            const response = await fetch(
                "/api/loginAttempt",
                {
                    method: "POST",
                    headers : {"Content-Type" : "application/json"},
                    body: JSON.stringify({login_submitted_email : email, login_submitted_password : password}),
                    redirect: "manual",
                }
            );
            if (response.status == 200) {
                console.log('breuh');
              navigate("/home");
            }
        } catch (err) {
            console.log(err);
        }
    };

    return (
        <div className="parent-container">
            <div className="login-parent">
                <div className='login-container'>
                    <h1 className="inter-font">Login</h1>

                    <div className="login-form inter-font">
                        <form onSubmit={handleLogin} id="login-form">
                            <div className='input-field'>
                                <h3 className='align-left'>Email</h3>
                                <input type="text" value={email} onChange={(e) => setEmail(e.target.value)} className="inter-font"></input>
                            </div>
                            
                            <div className='input-field'>
                                <h3 className='align-left'>Password</h3>
                                <input type="password" value={password} onChange={(e) => setPassword(e.target.value)} className="inter-font"></input>
                            </div>
                        </form>

                        <div className="buttons-and-links">
                            <div className='login-signup-button'>
                                <input type='submit' value='Login' form='login-form' id='login-button'></input>
                                <button onClick={() => {navigate("/registration")}} id='signup-button'>Sign Up</button>
                            </div>

                            <p><a onClick={() => {navigate("/forgot")}}>Forgot Password</a></p>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};
