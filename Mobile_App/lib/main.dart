import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:pbl4/ui/history.dart';
import 'package:pbl4/ui/home.dart';

import 'api/firebase_api.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  await FirebaseApi().initNotifications();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primaryColor: const Color(0xFF4CAF50),
        scaffoldBackgroundColor: const Color(0xFFE8F5E9),
      ),
      home: const TrashBinApp(),
    );
  }
}
