// Translated from solution.cpp.

var N: dynamic;

var X: dynamic;

var Y: dynamic;

var Z: dynamic;

var Data = cpp_array(100, 3);

func solve()
{
  var sum: dynamic;
  sum = 0;
  {
    var i = 0;
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

func main()
{
  var num: dynamic;
  memset(Data, false, cpp_sizeof((Data)));
  read(N, X);
  {
    var i = 0;
    while ((i < X))
    {
      read(num);
      Data[0][(num - 1)] = true;
      i += 1;
    }
  }
  read(Y);
  {
    var i = 0;
    while ((i < Y))
    {
      read(num);
      Data[1][(num - 1)] = true;
      i += 1;
    }
  }
  read(Z);
  {
    var i = 0;
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
