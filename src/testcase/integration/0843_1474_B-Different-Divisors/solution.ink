// Translated from solution.cpp.

var ll = dynamic;

var mod = cpp_expression("#include<b");

var v2: dynamic;

var prime = cpp_array(1000000);

func fun(n: dynamic)
{
  {
    var p = 2;
    while (((p * p) <= n))
    {
      if ((prime[p] == true))
      {
        {
          var i = (p * p);
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
    var p = 2;
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

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  read(t);
  memset(prime, true, cpp_sizeof((prime)));
  fun(1000000);
  sort(v2.begin(), v2.end());
  while (cpp_update(t, "--"))
  {
    var d: dynamic;
    var i: dynamic;
    read(d);
    var a = 1;
    var pro = 1;
    var cnt = 0;
    var ans2: dynamic;
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
