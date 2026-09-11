// Translated from solution.cpp.

var BIG_NUM: dynamic = cpp_expression("#include <");

var MOD: dynamic = cpp_expression("#include <");

var PRIME1: dynamic = cpp_expression("#include");

var PRIME2: dynamic = cpp_expression("#include");

var EPS: dynamic = cpp_expression("#include <");

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

func func_cpp() -> dynamic
{
  var baseTable: dynamic = cpp_array(N);
  var baseValue: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      scanf("%d", (&baseTable[i]));
      i += 1;
    }
  }
  var tmp: dynamic = 1;
  var pre: dynamic = cpp_uninitialized();
  {
    var p: dynamic = 0;
    while ((p < K))
    {
      tmp *= baseTable[p];
      p += 1;
    }
  }
  baseValue = tmp;
  pre = tmp;
  {
    var i: dynamic = 1;
    while ((i <= (N - K)))
    {
      tmp = (((pre / baseTable[(i - 1)])) * baseTable[((i + K) - 1)]);
      baseValue = max(baseValue, tmp);
      pre = tmp;
      i += 1;
    }
  }
  var nextValue: dynamic = 0;
  var tmpValue: dynamic = cpp_uninitialized();
  {
    var a: dynamic = 0;
    while ((a < (N - 1)))
    {
      {
        var b: dynamic = (a + 1);
        while ((b < N))
        {
          swap(baseTable[a], baseTable[b]);
          tmp = 1;
          {
            var p: dynamic = 0;
            while ((p < K))
            {
              tmp *= baseTable[p];
              p += 1;
            }
          }
          tmpValue = tmp;
          pre = tmp;
          {
            var i: dynamic = 1;
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

func main() -> dynamic
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
