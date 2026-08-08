// Translated from solution.cpp.

var n: dynamic;

var perm = cpp_array(110);

var viz = cpp_array(110);

var v = cpp_array(110);

func cmmdc(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return cmmdc(b, (a % b));
}

func solve()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(perm[i]);
      v[perm[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((v[i] != 1))
      {
        write(-1, "\n");
        return;
      }
      i += 1;
    }
  }
  var cycles: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!viz[i]))
      {
        viz[i] = true;
        var cnt = 1;
        {
          var j = perm[i];
          while ((j != i))
          {
            viz[j] = true;
            cnt += 1;
            j = perm[j];
          }
        }
        cycles.push_back(cnt);
      }
      i += 1;
    }
  }
  var cmmmc = 1;
  for (var el in cycles)
  {
    if (((el % 2) == 0))
    {
      el /= 2;
    }
    cmmmc = ((cmmmc * el) / cmmdc(el, cmmmc));
  }
  write(cmmmc, "\n");
}

func reset()
{
  {
    var i = 1;
    while ((i <= n))
    {
      v[i] = cpp_assign(viz[i], "=", 0);
      i += 1;
    }
  }
}

func main()
{
  var k = 1;
  {
    while ((k <= 1))
    {
      reset();
      solve();
      k += 1;
    }
  }
}
