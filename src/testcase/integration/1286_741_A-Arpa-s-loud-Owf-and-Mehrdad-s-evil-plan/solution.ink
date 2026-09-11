// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var perm: dynamic = cpp_array(110);

var viz: dynamic = cpp_array(110);

var v: dynamic = cpp_array(110);

func cmmdc(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return cmmdc(b, (a % b));
}

func solve() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(perm[i]);
      v[perm[i]] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
  var cycles: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!viz[i]))
      {
        viz[i] = true;
        var cnt: dynamic = 1;
        {
          var j: dynamic = perm[i];
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
  var cmmmc: dynamic = 1;
  for (var el: dynamic in cycles)
  {
    if (((el % 2) == 0))
    {
      el /= 2;
    }
    cmmmc = ((cmmmc * el) / cmmdc(el, cmmmc));
  }
  write(cmmmc, "\n");
}

func reset() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      v[i] = cpp_assign(viz[i], "=", 0);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var k: dynamic = 1;
  {
    while ((k <= 1))
    {
      reset();
      solve();
      k += 1;
    }
  }
}
