// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i = a; i < b; ++i)");
}

func REP(i: dynamic, N: dynamic)
{
  return cpp_expression("#include<i");
}

var N: dynamic;

var C: dynamic;

var D = cpp_array(30, 30);

var c = [0];

func main()
{
  scanf("%d%d", (&N), (&C));
  REP(i, C);
  REP(j, C);
  scanf("%d", (&D[i][j]));
  var buf: dynamic;
  REP(i, N);
  var minval = numeric_limits.max();
  REP(i, C);
  REP(j, C);
  if ((i != j))
  {
    if (((i != k) && (j != k)))
    {
      var val = ((c[0][i] + c[1][j]) + c[2][k]);
      if ((val < minval))
      {
        minval = val;
      }
    }
  }
  printf("%d\n", minval);
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    scanf("%d", (&buf));
    REP(k, C)[(((((i + 1)) + ((j + 1)))) % 3)][k] += D[(buf - 1)][k];
  }
