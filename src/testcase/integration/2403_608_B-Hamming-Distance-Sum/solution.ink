// Translated from solution.cpp.

var b1 = cpp_array(200010);

var b2 = cpp_array(200010);

func main(argument_0: dynamic)
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  memset(b1, 0, cpp_sizeof((b1)));
  memset(b2, 0, cpp_sizeof((b2)));
  var ans = 0;
  {
    var i = 0;
    while ((i < b.length()))
    {
      b1[i] = (b1[(i - 1)] + ((b[i] == cpp_char("1"))));
      b2[i] = (b2[(i - 1)] + ((b[i] == cpp_char("0"))));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < a.length()))
    {
      if ((a[i] == cpp_char("1")))
      {
        ans += ((b2[((b.length() - a.length()) + i)] - b2[(i - 1)]));
      } else
      {
        ans += ((b1[((b.length() - a.length()) + i)] - b1[(i - 1)]));
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
