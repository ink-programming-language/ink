// Translated from solution.cpp.

var str: dynamic = cpp_uninitialized();

func sim(turn: dynamic) -> dynamic
{
  var n: dynamic = str.size();
  var pos: dynamic = str[0];
  {
    var i: dynamic = 1;
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

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    read(str);
    var ans: dynamic = (sim(0) | sim(1));
    write(( (ans) ? "Yes" : "No"), "\n");
  }
  return 0;
}
