// Translated from solution.cpp.

var CRT_SECURE_NO_WARNINGS: dynamic = cpp_expression("#def");

var M: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

var MOD: dynamic = 1000000007;

func main() -> dynamic
{
  read(M);
  read(C);
  var N: dynamic = C.size();
  if (((N == 1) && (M == 0)))
  {
    write(0, "\n");
    return 0;
  }
  var ten: dynamic = 1;
  var arr: dynamic = [];
  {
    var i: dynamic = (N - 1);
    while ((i >= 0))
    {
      arr[(C[i] - cpp_char("0"))] += ten;
      arr[(C[i] - cpp_char("0"))] %= MOD;
      ten *= 10;
      ten %= MOD;
      i -= 1;
    }
  }
  var per: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < 10))
    {
      per.push_back(i);
      i += 1;
    }
  }
  var sent: dynamic = (C[0] - cpp_char("0"));
  while (true)
  {
    if ((per[sent] != 0))
    {
      var tmp: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < 10))
        {
          tmp += (((arr[i] * per[i])) % MOD);
          tmp %= MOD;
          i += 1;
        }
      }
      if ((tmp == M))
      {
        for (var c: dynamic in C)
        {
          write(per[(c - cpp_char("0"))]);
        }
        write("\n");
        return 0;
      }
    }
    if (!((next_permutation(per.begin(), per.end()))))
    {
      break;
    }
  }
  write(-1, "\n");
  return 0;
}
