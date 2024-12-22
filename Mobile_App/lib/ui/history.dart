import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/painting.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:intl/intl.dart';
import 'package:pbl4/model/TrashInformation.dart';
import 'package:pbl4/ui/detailTrash.dart';

class History extends StatefulWidget {

  const History({super.key});

  @override
  State<History> createState() => _HistoryState();
}

class _HistoryState extends State<History> with SingleTickerProviderStateMixin {
  List<TrashInformation> list = [];
  late AnimationController _controller;
  int currentIndex =  10;
  bool isLoading =  false;

  void loadMoredata()  {
    if(isLoading) return;
    setState(() {
      isLoading = true;
    });
  }

  @override
  void initState() {
    super.initState();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Colors.green,
        title: Text('Trash History'),
      ),
      body: StreamBuilder<List<TrashInformation>>(
          stream: fetchTrashInformation(),
          builder: (context, snapshot) {
            if(snapshot.hasError) {
              return Center(child: Text('Error: ${snapshot.error}'));
            }
            if (snapshot.connectionState == ConnectionState.waiting) {
              return Center(child: CircularProgressIndicator());
            }
            if (!snapshot.hasData || snapshot.data == null) {
              return Center(child: Text('No data available.'));
            }

             list = snapshot.data!;
             var displaylist = list.take(currentIndex).toList();

              return Column(
                children: [
                  Expanded(
                    child: ListView.builder(
                        itemCount: displaylist.length,
                        itemBuilder: (context, index) {
                          final trash = displaylist[index];
                          DateTime time = trash.timestamp;
                          String type = trash.type.toUpperCase();
                          String formattedTime = DateFormat('HH:mm dd MMM yyyy').format(time);
                          Color borderColor;
                          switch (type) {
                            case 'METAL':
                              borderColor = Colors.black;
                              break;
                            case 'PLASTIC':
                              borderColor = Colors.grey;
                              break;
                            case 'PAPER':
                              borderColor = Colors.green;
                              break;
                            default:
                              borderColor = Colors.red;
                              break;
                          }
                          return GestureDetector(
                            onTap: () async {
                              navigate(trash, borderColor);
                            },
                            child: Card(

                              elevation: 8,
                              shape: RoundedRectangleBorder(
                                side: BorderSide(color: borderColor, width: 4),
                                borderRadius: BorderRadius.circular(16),
                              ),
                              child: Row(
                                children: [
                                  Container(
                                    width: 120,
                                    height: 90,
                                    decoration: BoxDecoration(
                                      borderRadius: BorderRadius.circular(12.0),
                                      border: Border.all(color: Colors.grey.shade300, width: 2),
                                      color: Colors.grey.shade200,
                                    ),
                                    child: ClipRRect(
                                      borderRadius: BorderRadius.circular(8.0),
                                      child: Image.network(
                                        trash.imageURL,
                                        width: 300,
                                        height: 500,
                                        fit: BoxFit.cover,
                                        errorBuilder: (context, error, stackTrace) {
                                          return Image.asset(
                                            'assets/placeholder.png',
                                            width: 150,
                                            height: 300,
                                            fit: BoxFit.cover,
                                          );
                                        },
                                      ),
                                    ),
                                  ),
                                  Column(
                                    mainAxisAlignment: MainAxisAlignment.start,
                                    children: [
                                      Text('Classify: ' "$type", style: GoogleFonts
                                          .roboto(
                                        fontSize: 20,
                                        fontWeight: FontWeight.bold,
                                      ),),
                                      SizedBox(height: 5,),
                                      Text(formattedTime,
                                        softWrap: true,
                                        style: GoogleFonts.roboto(
                                          fontSize: 16,
                                        ),)
                                    ],
                                  )
                                ],
                              ),
                            ),
                          );

                        }
                    )
                  ),
                  if (currentIndex < list.length)
                    Padding(
                      padding: const EdgeInsets.all(8.0),
                      child: ElevatedButton(
                        onPressed: () {
                          setState(() {
                            currentIndex += 10; // Tải thêm 20 phần tử
                          });
                        },
                        child: isLoading
                            ? CircularProgressIndicator()
                            : Text('Continue',
                              style: TextStyle(fontSize: 24),),
                      ),
                    ),
                ],
              );

          }

      ),
    );
  }


  void navigate(TrashInformation trash, Color color) {
    Navigator.push(context, CupertinoPageRoute(builder: (context) {
      return DetailTrash(trashInformation: trash, color: color);
    }));
  }

}