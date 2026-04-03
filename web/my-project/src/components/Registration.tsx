import '../styling/Registration.css';
import { useState } from 'react';
import { useNavigate } from "react-router-dom";


export default function Registration() {
    const navigate = useNavigate();
    const [email, setEmail] = useState<string>("");
    const [password, setPassword] = useState<string>("");

    const handleRegistration = async (e : React.FormEvent<HTMLFormElement>) => {
        e.preventDefault()
        try {
            const response = await fetch(
                "/api/registration",
                {
                    method : "POST",
                    headers : {"Content-Type" : "application/json"},
                    body : JSON.stringify({registration_submitted_email : email, registration_submitted_password : password})
                }
            )
            if (response.status == 200) {
                navigate("/verification");
            }
        } catch (err) {
            console.log(err);
        }
    }

    return (
        <div className="registration-parent">
            <div className='registration-container'>
                <h1 className="inter-font">Sign Up</h1>

                <div className="registration-form inter-font">
                    <form onSubmit={handleRegistration} id="registration-form">
                        <div className='input-field'>
                            <h3 className='align-left'>Email</h3>
                            <input type="text" onChange={e => {setEmail(e.target.value)}} className="inter-font"></input>
                        </div>
                        
                        <div className='input-field'>
                            <h3 className='align-left'>Password</h3>
                            <input type="password" onChange={e => setPassword(e.target.value)} className="inter-font"></input>
                        </div>
                    </form>

                    <div className="send-email-button">
                        <input type='submit' value='Send Email' form='registration-form' id='registration-button'></input>
                    </div>
                </div>
            </div>
        </div>
    );
};