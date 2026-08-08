// Translated from solution.cpp.

var int_cpp = dynamic;

var INF = cpp_expression("#include<bi");

var MAX = cpp_expression("#inclu");

var N: dynamic;

var D: dynamic;

var t = cpp_array(MAX);

var F = cpp_array(MAX);

func main()
{
  var num = 0;
  var ans = 0;
  read(N, D);
  {
    var i = 0;
    while ((i < N))
    {
      read(t[i], F[i]);
      i += 1;
    }
  }
  t[N] = INF;
  F[N] = 1;
  if ((t[0] < (F[0] - 1)))
  {
    write(-1, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < N))
    {
      if (((num + 1) > D))
      {
        write(-1, "\n");
        return 0;
      } else
      {
        num += 1;
      }
      if (((t[(i + 1)] - t[i]) < abs((F[(i + 1)] - F[i]))))
      {
        write(-1, "\n");
        return 0;
      } else if (((t[(i + 1)] - t[i]) >= ((F[(i + 1)] + F[i]) - 2)))
      {
        ans += (num * ((F[i] - 1)));
        num = 0;
      } else
      {
        ans += (num * ((t[(i + 1)] - t[i])));
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
