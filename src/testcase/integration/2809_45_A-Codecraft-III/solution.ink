// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var s = ["0", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  var a: dynamic;
  read(a);
  var n: dynamic;
  var x: dynamic;
  read(n);
  {
    var i = 0;
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
  var b = (n % 12);
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
