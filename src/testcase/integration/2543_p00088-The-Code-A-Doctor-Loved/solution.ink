// Translated from solution.cpp.

var ab = ["100101", "10011010", "0101", "0001", "110", "01001", "10011011", "010000", "0111", "10011000", "0110", "00100", "10011001", "10011110", "00101", "111", "10011111", "1000", "00110", "00111", "10011100", "10011101", "000010", "10010010", "10010011", "10010000"];

func ctoi(c: dynamic)
{
  if ((c == cpp_char(" ")))
  {
    return "101";
  } else if ((c == cpp_char("'")))
  {
    return "000000";
  } else if ((c == cpp_char(",")))
  {
    return "000011";
  } else if ((c == cpp_char("-")))
  {
    return "10010001";
  } else if ((c == cpp_char(".")))
  {
    return "010001";
  } else if ((c == cpp_char("?")))
  {
    return "000001";
  } else if (((c >= cpp_char("A")) && (c <= cpp_char("Z"))))
  {
    return ab[(c - cpp_char("A"))];
  }
}

func itoc(num: dynamic)
{
  if ((num == "00000"))
  {
    return cpp_char("A");
  }
  if ((num == "00001"))
  {
    return cpp_char("B");
  }
  if ((num == "00010"))
  {
    return cpp_char("C");
  }
  if ((num == "00011"))
  {
    return cpp_char("D");
  }
  if ((num == "00100"))
  {
    return cpp_char("E");
  }
  if ((num == "00101"))
  {
    return cpp_char("F");
  }
  if ((num == "00110"))
  {
    return cpp_char("G");
  }
  if ((num == "00111"))
  {
    return cpp_char("H");
  }
  if ((num == "01000"))
  {
    return cpp_char("I");
  }
  if ((num == "01001"))
  {
    return cpp_char("J");
  }
  if ((num == "01010"))
  {
    return cpp_char("K");
  }
  if ((num == "01011"))
  {
    return cpp_char("L");
  }
  if ((num == "01100"))
  {
    return cpp_char("M");
  }
  if ((num == "01101"))
  {
    return cpp_char("N");
  }
  if ((num == "01110"))
  {
    return cpp_char("O");
  }
  if ((num == "01111"))
  {
    return cpp_char("P");
  }
  if ((num == "10000"))
  {
    return cpp_char("Q");
  }
  if ((num == "10001"))
  {
    return cpp_char("R");
  }
  if ((num == "10010"))
  {
    return cpp_char("S");
  }
  if ((num == "10011"))
  {
    return cpp_char("T");
  }
  if ((num == "10100"))
  {
    return cpp_char("U");
  }
  if ((num == "10101"))
  {
    return cpp_char("V");
  }
  if ((num == "10110"))
  {
    return cpp_char("W");
  }
  if ((num == "10111"))
  {
    return cpp_char("X");
  }
  if ((num == "11000"))
  {
    return cpp_char("Y");
  }
  if ((num == "11001"))
  {
    return cpp_char("Z");
  }
  if ((num == "11010"))
  {
    return cpp_char(" ");
  }
  if ((num == "11011"))
  {
    return cpp_char(".");
  }
  if ((num == "11100"))
  {
    return cpp_char(",");
  }
  if ((num == "11101"))
  {
    return cpp_char("-");
  }
  if ((num == "11110"))
  {
    return cpp_char("'");
  }
  if ((num == "11111"))
  {
    return cpp_char("?");
  }
}

func main()
{
  var str: dynamic;
  while (getline(cin, str))
  {
    var num = "";
    {
      var i = 0;
      while ((i < str.length()))
      {
        num += ctoi(str[i]);
        i += 1;
      }
    }
    var cnt = 0;
    var temp = "";
    var res = "";
    {
      var i = 0;
      while ((i < num.length()))
      {
        cnt += 1;
        temp += num[i];
        if ((cnt == 5))
        {
          res += itoc(temp);
          cnt = 0;
          temp = "";
        }
        i += 1;
      }
    }
    if ((temp.length() > 0))
    {
      while ((temp.length() < 5))
      {
        temp += cpp_char("0");
      }
      res += itoc(temp);
    }
    write(res, "\n");
  }
}
