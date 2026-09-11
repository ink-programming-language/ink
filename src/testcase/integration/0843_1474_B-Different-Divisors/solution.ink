// Translated from solution.cpp.

var ll: dynamic = dynamic;

var mod: dynamic = cpp_expression("#include<b");

var v2: dynamic = cpp_uninitialized();

var prime: dynamic = cpp_array(1000000);

func fun(n: dynamic) -> dynamic
{
  {
    var p: dynamic = 2;
    while (((p * p) <= n))
    {
      if ((prime[p] == true))
      {
        {
          var i: dynamic = (p * p);
          while ((i <= n))
          {
            prime[i] = false;
            i += p;
          }
        }
      }
      p += 1;
    }
  }
  {
    var p: dynamic = 2;
    while ((p <= n))
    {
      if (prime[p])
      {
        v2.push_back(p);
      }
      p += 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  memset(prime, true, cpp_sizeof((prime)));
  fun(1000000);
  sort(v2.begin(), v2.end());
  while (cpp_update(t, "--"))
  {
    var d: dynamic = cpp_uninitialized();
    var i: dynamic = cpp_uninitialized();
    read(d);
    var a: dynamic = 1;
    var pro: dynamic = 1;
    var cnt: dynamic = 0;
    var ans2: dynamic = cpp_uninitialized();
    {
      i = (1 + d);
      while ((i < v2.size()))
      {
        if ((prime[i] == true))
        {
          cnt = i;
          break;
        }
        i += 1;
      }
    }
    {
      i = (cnt + d);
      while ((i < v2.size()))
      {
        if ((prime[i] == true))
        {
          ans2 = i;
          break;
        }
        i += 1;
      }
    }
    write((ans2 * cnt), cpp_char("\n"));
  }
}
