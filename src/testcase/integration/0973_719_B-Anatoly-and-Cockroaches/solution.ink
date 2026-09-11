// Translated from solution.cpp.

var maxn: dynamic = (3e6 + 5);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  var swap: dynamic = [0];
  var color: dynamic = [0];
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((((i % 2) == 0) && (s[i] == cpp_char("r"))))
      {
        color[0][0] += 1;
      }
      if ((((i % 2) == 1) && (s[i] == cpp_char("b"))))
      {
        color[0][1] += 1;
      }
      if ((((i % 2) == 0) && (s[i] == cpp_char("b"))))
      {
        color[1][0] += 1;
      }
      if ((((i % 2) == 1) && (s[i] == cpp_char("r"))))
      {
        color[1][1] += 1;
      }
      i += 1;
    }
  }
  write(min(max(color[0][0], color[0][1]), max(color[1][0], color[1][1])), "\n");
}
