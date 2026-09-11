// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var s: dynamic = ["0", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  var a: dynamic = cpp_uninitialized();
  read(a);
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < 13))
    {
      if ((a == s[i]))
      {
        x = i;
        break;
      }
      i += 1;
    }
  }
  var b: dynamic = (n % 12);
  if ((b == 0))
  {
    write(s[x], "\n");
  } else if (((x + b) > 12))
  {
    b = (((x + b)) % 12);
    write(s[b], "\n");
  } else
  {
    write(s[(x + b)], "\n");
  }
}
