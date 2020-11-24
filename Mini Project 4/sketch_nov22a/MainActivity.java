package com.example.iotfirebase;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.ToggleButton;


import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {

    ToggleButton btnServo;

    TextView suhu;
    TextView ldr;

    String valueSuhu;
    String valueLdr;
    String valueServo;

    DatabaseReference dref;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnServo = (ToggleButton) findViewById(R.id.toggleButton_status);

        suhu = (TextView) findViewById(R.id.textView_suhu);
        ldr = (TextView) findViewById(R.id.textView_ldr);

        dref = FirebaseDatabase.getInstance().getReference();
        dref.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                valueLdr = dataSnapshot.child("Node1/ldr").getValue().toString();
                ldr.setText(valueLdr);

                valueSuhu = dataSnapshot.child("Node1/suhu").getValue().toString();
                suhu.setText(valueSuhu);

                valueServo = dataSnapshot.child("Node1/servo").getValue().toString();
                if(valueServo.equals("0")) {
                    btnServo.setChecked(false);
                }else {
                    btnServo.setChecked(true);
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {

            }
        });

        btnServo.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(isChecked){
                    DatabaseReference servoRef = FirebaseDatabase.getInstance().getReference("Node1/servo");
                    servoRef.setValue(1);
                }else{
                    DatabaseReference servoRef = FirebaseDatabase.getInstance().getReference("Node1/servo");
                    servoRef.setValue(0);
                }
            }
        });
    }
}