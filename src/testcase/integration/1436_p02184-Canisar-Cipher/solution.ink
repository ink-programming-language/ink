// Translated from solution.cpp.

var CRT_SECURE_NO_WARNINGS = cpp_expression("#def");

var M: dynamic;

var C: dynamic;

var MOD = 1000000007;

func main()
{
  read(M);
  read(C);
  var N = C.size();
  if (((N == 1) && (M == 0)))
  {
    write(0, "\n");
    return 0;
  }
  var ten = 1;
  var arr = [];
  {
    var i = (N - 1);
    while ((i >= 0))
    {
      arr[(C[i] - cpp_char("0"))] += ten;
      arr[(C[i] - cpp_char("0"))] %= MOD;
      ten *= 10;
      ten %= MOD;
      i -= 1;
    }
  }
  var per: dynamic;
  {
    var i = 0;
    while ((i < 10))
    {
      per.push_back(i);
      i += 1;
    }
  }
  var sent = (C[0] - cpp_char("0"));
  while (true)
  {
    if ((per[sent] != 0))
    {
      var tmp = 0;
      {
        var i = 0;
        while ((i < 10))
        {
          tmp += (((arr[i] * per[i])) % MOD);
          tmp %= MOD;
          i += 1;
        }
      }
      if ((tmp == M))
      {
        for (var c in C)
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
