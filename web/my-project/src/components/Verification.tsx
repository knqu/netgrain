import '../styling/Verification.css';
import { useState } from 'react';
import { useNavigate } from "react-router-dom";

export default function Verification() {
    const navigate = useNavigate();
    const [code, setCode] = useState<string>("");
    const verify = async (e : React.FormEvent<HTMLFormElement>) => {
        e.preventDefault()
        try {
            const response = await fetch(
                "/api/verifyEmail",
                {
                    method : "POST",
                    headers : {"Content-Type" : "application/json"},
                    body : JSON.stringify({submitted_verification_code : code})
                }
            )
            if (response.status == 200) {
                navigate("/home");
            }
            else {
                navigate("/")
            }
        } catch (err) {
            console.log(err);
        }
    }
    return (
        <div className="registration-parent">
            <div className='registration-container'>
                <h1 className="inter-font">Verify Email</h1>

                <div className="registration-form inter-font">
                    <form onSubmit={verify} id="verification-form">
                        <div className='input-field'>
                            <h3 className='align-left'>Verification Code</h3>
                            <input type="text" onChange={e => setCode(e.target.value)} className="inter-font"></input>
                        </div>
                    </form>
                    <div className="send-email-button">
                        <input type='submit' value='Verify' form='verification-form' id='registration-button'></input>
                    </div>
                </div>
            </div>
        </div>
    );
};