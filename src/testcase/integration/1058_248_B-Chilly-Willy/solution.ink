// Translated from solution.cpp.

var N = 100005;

var vmod: dynamic;

var ans: dynamic;

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  vmod.push_back(0);
  vmod.push_back(0);
  vmod.push_back(0);
  vmod.push_back(100);
  i = 4;
  while ((i < N))
  {
    vmod.push_back((((vmod[(i - 1)] * 10)) % 210));
    i += 1;
  }
  var n: dynamic;
  var x: dynamic;
  read(n);
  if ((!vmod[n]))
  {
    write(-1, "\n");
  } else
  {
    ans.push_back(1);
    {
      i = 1;
      while ((i < n))
      {
        ans.push_back(0);
        i += 1;
      }
    }
    x = (210 - vmod[i]);
    j = (n - 1);
    while (x)
    {
      ans[j] = (ans[j] + (x % 10));
      x = (x / 10);
      j -= 1;
    }
    {
      j = 0;
      while ((j < ans.size()))
      {
        write(ans[j]);
        j += 1;
      }
    }
  }
  return 0;
}
