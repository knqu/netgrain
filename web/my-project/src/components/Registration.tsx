import '../styling/Registration.css';

function Registration() {
    return (
        <div className="registration-parent">
            <div className='registration-container'>
                <h1 className="inter-font">Sign Up</h1>

                <div className="registration-form inter-font">
                    <form action="/signupAttempt" method="get" id="registration-form">
                        <div className='input-field'>
                            <h3 className='align-left'>Email</h3>
                            <input type="text" name="registration_submitted_email" className="inter-font"></input>
                        </div>
                        
                        <div className='input-field'>
                            <h3 className='align-left'>Password</h3>
                            <input type="password" name="registration_submitted_password" className="inter-font"></input>
                        </div>
                    </form>

                    <div className="send-email-button">
                        <input type='submit' value='Send Email' form='registration-form' id='registration-button'></input>
                    </div>
                </div>
            </div>
        </div>
    );
}

export default Registration;