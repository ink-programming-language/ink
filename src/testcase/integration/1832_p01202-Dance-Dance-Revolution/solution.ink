// Translated from solution.cpp.

var str: dynamic;

func sim(turn: dynamic)
{
  var n = str.size();
  var pos = str[0];
  {
    var i = 1;
    while ((i < n))
    {
      if ((pos == str[i]))
      {
        return 0;
      }
      if (((turn == 1) && (str[i] == cpp_char("R"))))
      {
        return 0;
      }
      if (((turn == 0) && (str[i] == cpp_char("L"))))
      {
        return 0;
      }
      pos = str[i];
      turn = (!turn);
      i += 1;
    }
  }
  return 1;
}

func main()
{
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    read(str);
    var ans = (sim(0) | sim(1));
    write((if (ans) "Yes" else "No"), "\n");
  }
  return 0;
}
