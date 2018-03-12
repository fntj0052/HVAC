package com.mhsm0034.fntj0052.hvac;

import android.content.Intent;
import android.net.Uri;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Patterns;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class RegisterActivity extends AppCompatActivity {
    private EditText cname, cmail, cpass, ccpass;
    private String name, email, password, cpassword;
    Button csubmit;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register);
        cname = (EditText)findViewById(R.id.editNameText);
        cmail = (EditText)findViewById(R.id.editEmailText);
        cpass = (EditText) findViewById(R.id.editPasswordText);
        ccpass = (EditText) findViewById(R.id.editConfrimText);
        csubmit = (Button)findViewById(R.id.buttonConfirm);
        csubmit.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View View){
                register();
            }
        });
    }
    public void register(){
        initialize();
        if(!validate()){
            Toast.makeText(this,"Sign-Up Failed",Toast.LENGTH_SHORT).show();
        }
        else {
            onSignupSuccess();
        }
    }

    public  void onSignupSuccess(){
        // TODO after the valid input

    }

    public boolean validate(){
        boolean valid = true;
        if(name.isEmpty() ||name.length()>32){
            cname.setError("Please Enter Valid Name");
            valid = false;
        }
        if (email.isEmpty() ||!Patterns.EMAIL_ADDRESS.matcher(email).matches()){
            cmail.setError("Please Enter Valid E-mail");
            valid = false;
        }

        if (password.isEmpty()){
            cpass.setError("Please Enter Password");
            valid = false;
        }
        if (cpassword != password){
            ccpass.setError("Please Re-enter Password");
            valid = false;
        }
        return valid;
    }

    public void initialize(){
        name = cname.getText().toString().trim();
        email = cmail.getText().toString().trim();
        password = cpass.getText().toString().trim();
        cpassword = ccpass.getText().toString().trim();
    }

    public void back(View view){
        Intent intent = new Intent(this, LogInActivity.class);
        startActivity(intent);
    }

    public void onBackPressed() {
        moveTaskToBack(true);
    }

    @Override
    public  boolean onCreateOptionsMenu(Menu menu){
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.nav_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(final MenuItem item) {

        switch (item.getItemId()) {
            case R.id.home:
                Intent intent = new Intent(this, MainActivity.class);
                startActivity(intent);
                return true;
            case R.id.login:
                Intent intent1 = new Intent(this, LogInActivity.class);
                startActivity(intent1);
                return true;
            case R.id.register:
                Intent intent2 = new Intent(this, RegisterActivity.class);
                startActivity(intent2);
                return true;
            case R.id.status:
                Intent intent3 = new Intent(this, StatusActivity.class);
                startActivity(intent3);
                return true;
            case R.id.sound:
                Intent intent4 = new Intent(this, SoundActivity.class);
                startActivity(intent4);
                return true;
            case R.id.about:
                String url ="";
                Intent intent5 = new Intent(Intent.ACTION_VIEW);
                intent5.setData(Uri.parse(url));
                startActivity(intent5);
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
}
