// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var INF: dynamic = cpp_expression("#include<bi");

var MAX: dynamic = cpp_expression("#inclu");

var N: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(MAX);

var F: dynamic = cpp_array(MAX);

func main() -> dynamic
{
  var num: dynamic = 0;
  var ans: dynamic = 0;
  read(N, D);
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
