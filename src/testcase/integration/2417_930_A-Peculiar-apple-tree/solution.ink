// Translated from solution.cpp.

var arr = cpp_array((100000 + 100));

func main()
{
  var n: dynamic;
  read(n);
  var v = cpp_array((n + 1));
  {
    var i = 2;
    while ((i <= n))
    {
      var t: dynamic;
      read(t);
      v[t].push_back(i);
      v[i].push_back(t);
      i += 1;
    }
  }
  var q: dynamic;
  q.push(1);
  arr[1] = 1;
  var max = 1;
  var x = 0;
  var k = 0;
  var coun = 1;
  while ((!q.empty()))
  {
    var t = q.front();
    q.pop();
    {
      var i = 0;
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
