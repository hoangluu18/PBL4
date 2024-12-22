

import 'package:awesome_snackbar_content/awesome_snackbar_content.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:pbl4/ui/history.dart';

import 'package:pbl4/model/trashBin.dart';



class TrashBinApp extends StatelessWidget {
  const TrashBinApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Trash App',
      theme: ThemeData(
          colorScheme: ColorScheme.fromSeed(seedColor: Colors.green),
          useMaterial3: true),
      home: const Home(),
      debugShowCheckedModeBanner: false,
    );
  }
}


class Home extends StatefulWidget {
  const Home({super.key});

  @override
  State<Home> createState() => _HomeState();
}

class _HomeState extends State<Home> {
  final List<Widget> _tabs = [
    const HomePage(),
    const History(),
  ];
  @override
  Widget build(BuildContext context) {
    return CupertinoPageScaffold(

        child: CupertinoTabScaffold(
          tabBar: CupertinoTabBar(
            backgroundColor: Theme.of(context).colorScheme.onInverseSurface,
            items: const [
              BottomNavigationBarItem(icon: Icon(Icons.home), label: 'Home'),
              BottomNavigationBarItem(icon: Icon(Icons.history), label: 'History'),
            ],
          ),
          tabBuilder: (BuildContext context, int index) {
            return _tabs[index];
          },
        )
    );
  }
}



class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  List<Trashbin> trashbins = [
    Trashbin(
        imageURL: 'assets/other.png',
        count: 0,
        emptiness: false,
        bin: "other"),
    Trashbin(
        imageURL: 'assets/metal-trash.png',
        count: 0,
        emptiness: false,
        bin: "metal"),
    Trashbin(
        imageURL: 'assets/paper-bin.png',
        count: 0,
        emptiness: false,
        bin: "paper"),
    Trashbin(
        imageURL: 'assets/plastic.png',
        count: 0,
        emptiness: false,
        bin: "plastic"),
  ];
  @override
  void initState() {
    super.initState();
    for (int i = 0; i < 4; i++) {
      trashbins[i].listenToEmptinessUpdates(() {
        setState(() {}); // Cập nhật UI khi dữ liệu thay đổi
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: const Text('Trash Bin'),
          backgroundColor: Color(0xFF4CAF50),
        ),
        body: buildTrashCounter(trashbins)
        );
  }



  Widget buildTrashCounter(List<Trashbin> trashBin) {
    return Scaffold(
        backgroundColor: const Color(0xFFC8E6C9),
        body: Column(
          mainAxisAlignment: MainAxisAlignment.center,
            children: [
          Row(
            children: [
              TrashbinCard(context, trashBin[0]),
              const SizedBox(
                width: 16,
              ),
              TrashbinCard(context, trashBin[1])
            ],
          ),
          const SizedBox(
            height: 15,
          ),
          Row(
            children: [
              TrashbinCard(context, trashBin[2]),
              const SizedBox(
                width: 16,
              ),
              TrashbinCard(context, trashBin[3])
            ],
          )
        ]));
  }

  Widget TrashbinCard(BuildContext context, Trashbin trashBin) {
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (trashBin.emptiness) {
        final snackBar = SnackBar(
          elevation: 0,
          behavior: SnackBarBehavior.floating,
          backgroundColor: Colors.transparent,
          content: AwesomeSnackbarContent(
            title: 'Warning!',
            message: 'The ${trashBin.bin} trash bin is full.',
            contentType: ContentType.warning, // Có thể đổi sang success, failure, help
          ),
        );
        ScaffoldMessenger.of(context).showSnackBar(snackBar);
      }
    });
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 15.0),
      child: Card(
        elevation: 8,
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(16),
        ),
        child: Stack(
          children: [
            Container(
              width: 175,
              height: 270,
              //alignment: Alignment.center,
              padding: const EdgeInsets.all(16),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  Text(
                    trashBin.bin.toUpperCase(),
                    style:
                    const TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
                    textAlign: TextAlign.center,
                  ),
                  // Hình ảnh PNG
                  Expanded(
                    child: Image.asset(
                      height: 100,
                      trashBin.imageURL, // Đường dẫn đến hình ảnh
                      fit: BoxFit.contain, // Điều chỉnh hình ảnh vừa khung
                      errorBuilder: (context, error, stackTrace) {
                        return const Icon(
                          Icons.error, // Icon thay thế khi hình ảnh không tải được
                          size: 50,
                          color: Colors.red,
                        );
                      },
                    ),
                  ),
                  const SizedBox(height: 8,),
                  Text(
                    'Count: ' + trashBin.count.toString(),
                    style: GoogleFonts.roboto(
                        textStyle: const TextStyle(
                            fontSize: 18,
                            fontWeight: FontWeight.bold,)
                    ),
                  ),
                  const SizedBox(height: 8,),
                  Text(
                    trashBin.emptiness ? 'FULL' : 'NOT FULL',
                    style: GoogleFonts.roboto(
                        textStyle: TextStyle(
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                            color: trashBin.emptiness ? Colors.green : Colors.red)),
                  )
                ],
              ),
            ),
          ],
        )
      ),
    );
  }
}

