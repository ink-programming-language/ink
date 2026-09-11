// Translated from solution.cpp.

var llint: dynamic = dynamic;

var n: dynamic = cpp_uninitialized();

var dif: dynamic = cpp_array(100005);

func main(argument_0: dynamic) -> dynamic
{
  read(n);
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(l, r);
      dif[l] += 1;
      dif[(r + 1)] -= 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 100005))
    {
      dif[i] += dif[(i - 1)];
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      if ((dif[(i + 1)] >= i))
      {
        ans = i;
        break;
      }
      i -= 1;
    }
  }
  write(ans, "\n");
  return 0;
}
