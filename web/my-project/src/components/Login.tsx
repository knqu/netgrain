import '../styling/Login.css';

function Login() {
    return (
        <div className="login-parent">
            <div className='login-container'>
                <h1 className="inter-font">Login</h1>

                <div className="login-form inter-font">
                    <form action="/loginAttempt" method="get" id="login-form">
                        <div className='input-field'>
                            <h3 className='align-left'>Email</h3>
                            <input type="text" name="login_submitted_email" className="inter-font"></input>
                        </div>
                        
                        <div className='input-field'>
                            <h3 className='align-left'>Password</h3>
                            <input type="password" name="login_submitted_password" className="inter-font"></input>
                        </div>
                    </form>

                    <div className="buttons-and-links">
                        <div className='login-signup-button'>
                            <input type='submit' value='Login' form='login-form' id='login-button'></input>
                            <input type='button' value='Sign Up' id='signup-button'></input>
                        </div>

                        <p><a>Forgot Password</a></p>
                    </div>
                </div>
            </div>
        </div>
    );
}

export default Login;