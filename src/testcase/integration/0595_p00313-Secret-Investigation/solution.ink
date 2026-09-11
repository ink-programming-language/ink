// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

var Z: dynamic = cpp_uninitialized();

var Data: dynamic = cpp_array(100, 3);

func solve() -> dynamic
{
  var sum: dynamic = cpp_uninitialized();
  sum = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if (((((!Data[0][i]) && Data[2][i])) || ((Data[1][i] && Data[2][i]))))
      {
        sum += 1;
      }
      i += 1;
    }
  }
  write(sum, "\n");
}

func main() -> dynamic
{
  var num: dynamic = cpp_uninitialized();
  memset(Data, false, cpp_sizeof((Data)));
  read(N, X);
  {
    var i: dynamic = 0;
    while ((i < X))
    {
      read(num);
      Data[0][(num - 1)] = true;
      i += 1;
    }
  }
  read(Y);
  {
    var i: dynamic = 0;
    while ((i < Y))
    {
      read(num);
      Data[1][(num - 1)] = true;
      i += 1;
    }
  }
  read(Z);
  {
    var i: dynamic = 0;
    while ((i < Z))
    {
      read(num);
      Data[2][(num - 1)] = true;
      i += 1;
    }
  }
  solve();
  return 0;
}
