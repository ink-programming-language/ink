// Translated from solution.cpp.

func read(args: dynamic...)
{
  cpp_fold("((cin >> args), ...)");
}

func write(args: dynamic...)
{
  cpp_fold("((cout << args << \" \"), ...)");
}

func writeln(args: dynamic...)
{
  cpp_fold("((cout << args << \" \"), ...)");
  write("\n");
}

func read(a: dynamic)
{
  for (var ele in a)
  {
    read(ele);
  }
}

func writeln(a: dynamic)
{
  for (var ele in a)
  {
    write(ele, cpp_char(" "));
  }
  write("\n");
}

var dxy = [[-1, 0], [1, 0], [0, -1], [0, 1]];

var fxy = [[-1, 0], [1, 0], [0, -1], [0, 1], [1, 1], [-1, -1], [-1, 1], [1, -1]];

func main()
{
  cin.tie(0);
  cout.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  var m: dynamic;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var tmp = s.substr(i, 2);
      m[tmp] += 1;
      i += 1;
    }
  }
  var cnt = 0;
  var res: dynamic;
  {
    var it = m.begin();
    while ((it != m.end()))
    {
      if ((it->second > cnt))
      {
        cnt = it->second;
        res = it->first;
      }
      it += 1;
    }
  }
  writeln(res);
  return 0;
}
