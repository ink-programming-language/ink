// Translated from solution.cpp.

var llint = dynamic;

var n: dynamic;

var dif = cpp_array(100005);

func main(argument_0: dynamic)
{
  read(n);
  var l: dynamic;
  var r: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      read(l, r);
      dif[l] += 1;
      dif[(r + 1)] -= 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < 100005))
    {
      dif[i] += dif[(i - 1)];
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = n;
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
