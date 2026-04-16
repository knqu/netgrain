import '../styling/ForgotPassword.css';
import { useState } from 'react';
import { useNavigate } from "react-router-dom";

export default function ForgotPassword() {
    const navigate = useNavigate();
    const [email, setEmail] = useState<string>("");
    const [password, setPassword] = useState<string>("");
    const verify = async (e : React.FormEvent<HTMLFormElement>) => {
        e.preventDefault()
        try {
            const response = await fetch(
                "http://localhost:18080/api/forgotPassword",
                {
                    method : "POST",
                    headers : {"Content-Type" : "application/json"},
                    body : JSON.stringify({submitted_email : email, submitted_new_password : password})
                }
            )
            if (response.status == 200) {
                navigate("/verification");
            }
            else {
                navigate("/")
            }
        } catch (err) {
            console.log(err);
        }
    }
    return (
        <div className="parent-container">
            <div className="registration-parent">
                <div className='registration-container'>
                    <h1 className="inter-font">Reset Password</h1>

                    <div className="registration-form inter-font">
                        <form onSubmit={verify} id="verification-form">
                            <div className='input-field'>
                                <h3 className='align-left'>Email</h3>
                                <input type="text" onChange={e => setEmail(e.target.value)} className="inter-font"></input>
                            </div>

                            <div className='input-field'>
                                <h3 className='align-left'>New Password</h3>
                                <input type="password" onChange={e => setPassword(e.target.value)} className="inter-font"></input>
                            </div>
                        </form>
                        <div className="send-email-button">
                            <input type='submit' value='Verify' form='verification-form' id='registration-button'></input>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};