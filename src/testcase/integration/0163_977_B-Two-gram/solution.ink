// Translated from solution.cpp.

func read(args: dynamic...) -> dynamic
{
  cpp_fold("((cin >> args), ...)");
}

func write(args: dynamic...) -> dynamic
{
  cpp_fold("((cout << args << \" \"), ...)");
}

func writeln(args: dynamic...) -> dynamic
{
  cpp_fold("((cout << args << \" \"), ...)");
  write("\n");
}

func read(a: dynamic) -> dynamic
{
  for (var ele: dynamic in a)
  {
    read(ele);
  }
}

func writeln(a: dynamic) -> dynamic
{
  for (var ele: dynamic in a)
  {
    write(ele, cpp_char(" "));
  }
  write("\n");
}

var dxy: dynamic = [[-1, 0], [1, 0], [0, -1], [0, 1]];

var fxy: dynamic = [[-1, 0], [1, 0], [0, -1], [0, 1], [1, 1], [-1, -1], [-1, 1], [1, -1]];

func main() -> dynamic
{
  cin.tie(0);
  cout.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  var m: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var tmp: dynamic = s.substr(i, 2);
      m[tmp] += 1;
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  var res: dynamic = cpp_uninitialized();
  {
    var it: dynamic = m.begin();
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
