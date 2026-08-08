// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var MOD = 1000000007;

var MAXX = 5010;

var number = cpp_array(MAXX, MAXX);

var acum = cpp_array(MAXX, MAXX);

var x: dynamic;

var y: dynamic;

func solve()
{
  var ret = 0;
  memset(number, 0, cpp_sizeof((number)));
  memset(acum, 0, cpp_sizeof((acum)));
  {
    var i = 0;
    while ((i < x.size()))
    {
      number[i][0] = cpp_assign(acum[i][0], "=", ((x[i] == y[0])));
      ret = (((ret + number[i][0])) % MOD);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < y.size()))
    {
      number[0][i] = cpp_assign(acum[0][i], "=", ((y[i] == x[0])));
      acum[0][i] += acum[0][(i - 1)];
      ret = (((ret + number[0][i])) % MOD);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < x.size()))
    {
      {
        var j = 1;
        while ((j < y.size()))
        {
          if ((x[i] == y[j]))
          {
            number[i][j] = (((number[i][j] + acum[(i - 1)][(j - 1)])) % MOD);
            number[i][j] += 1;
            number[i][j] %= MOD;
          }
          ret = (((ret + number[i][j])) % MOD);
          acum[i][j] = (((number[i][j] + acum[i][(j - 1)])) % MOD);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ret;
}

func main()
{
  read(x, y);
  write(solve(), "\n");
  return 0;
}
