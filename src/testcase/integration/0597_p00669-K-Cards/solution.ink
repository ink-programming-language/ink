// Translated from solution.cpp.

var BIG_NUM = cpp_expression("#include <");

var MOD = cpp_expression("#include <");

var PRIME1 = cpp_expression("#include");

var PRIME2 = cpp_expression("#include");

var EPS = cpp_expression("#include <");

var N: dynamic;

var K: dynamic;

func func_cpp()
{
  var baseTable = cpp_array(N);
  var baseValue = 0;
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%d", (&baseTable[i]));
      i += 1;
    }
  }
  var tmp = 1;
  var pre: dynamic;
  {
    var p = 0;
    while ((p < K))
    {
      tmp *= baseTable[p];
      p += 1;
    }
  }
  baseValue = tmp;
  pre = tmp;
  {
    var i = 1;
    while ((i <= (N - K)))
    {
      tmp = (((pre / baseTable[(i - 1)])) * baseTable[((i + K) - 1)]);
      baseValue = max(baseValue, tmp);
      pre = tmp;
      i += 1;
    }
  }
  var nextValue = 0;
  var tmpValue: dynamic;
  {
    var a = 0;
    while ((a < (N - 1)))
    {
      {
        var b = (a + 1);
        while ((b < N))
        {
          swap(baseTable[a], baseTable[b]);
          tmp = 1;
          {
            var p = 0;
            while ((p < K))
            {
              tmp *= baseTable[p];
              p += 1;
            }
          }
          tmpValue = tmp;
          pre = tmp;
          {
            var i = 1;
            while ((i <= (N - K)))
            {
              tmp = (((pre / baseTable[(i - 1)])) * baseTable[((i + K) - 1)]);
              tmpValue = max(tmpValue, tmp);
              pre = tmp;
              i += 1;
            }
          }
          nextValue = max(nextValue, tmpValue);
          swap(baseTable[a], baseTable[b]);
          b += 1;
        }
      }
      a += 1;
    }
  }
  if ((nextValue < baseValue))
  {
    printf("NO GAME\n");
  } else
  {
    printf("%d\n", (nextValue - baseValue));
  }
}

func main()
{
  while (true)
  {
    scanf("%d %d", (&N), (&K));
    if (((N == 0) && (K == 0)))
    {
      break;
    }
    func_cpp();
  }
  return 0;
}
