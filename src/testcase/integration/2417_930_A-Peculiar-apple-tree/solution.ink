// Translated from solution.cpp.

var arr: dynamic = cpp_array((100000 + 100));

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var v: dynamic = cpp_array((n + 1));
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      var t: dynamic = cpp_uninitialized();
      read(t);
      v[t].push_back(i);
      v[i].push_back(t);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  q.push(1);
  arr[1] = 1;
  var max: dynamic = 1;
  var x: dynamic = 0;
  var k: dynamic = 0;
  var coun: dynamic = 1;
  while ((!q.empty()))
  {
    var t: dynamic = q.front();
    q.pop();
    {
      var i: dynamic = 0;
      while ((i < v[t].size()))
      {
        if ((arr[v[t][i]] == 0))
        {
          q.push(v[t][i]);
          arr[v[t][i]] = 1;
          x += 1;
        }
        i += 1;
      }
    }
    k += 1;
    if ((k == coun))
    {
      if (((x % 2) != 0))
      {
        max += 1;
      }
      coun = x;
      x = 0;
      k = 0;
    }
  }
  write(max);
}
