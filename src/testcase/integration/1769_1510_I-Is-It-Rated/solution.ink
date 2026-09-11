// Translated from solution.cpp.

var miss: dynamic = cpp_array(1005);

var war: dynamic = cpp_array(1005);

func losuj(a: dynamic) -> dynamic
{
  var res: dynamic = 1;
  {
    var x: dynamic = 1;
    while ((x <= 10))
    {
      res = ((((res * rand())) % a) + 1);
      x += 1;
    }
  }
  return res;
}

func main() -> dynamic
{
  srand((time(null) + clock()));
  var a: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(a, t);
  while (cpp_update(t, "--"))
  {
    var d: dynamic = cpp_uninitialized();
    read(d);
    var mini: dynamic = 1e9;
    var maks: dynamic = -1e9;
    {
      var x: dynamic = 0;
      while ((x < a))
      {
        maks = max(maks, miss[x]);
        mini = min(mini, miss[x]);
        x += 1;
      }
    }
    if (((maks - mini) > 110))
    {
      maks = (mini + 110);
    }
    var sum: dynamic = 0;
    {
      var x: dynamic = 0;
      while ((x < a))
      {
        if ((miss[x] > maks))
        {
          war[x] = 0;
        } else
        {
          war[x] = (1 << ((((maks - miss[x])) / 4)));
        }
        sum += war[x];
        x += 1;
      }
    }
    sum = losuj(sum);
    var co: dynamic = -1;
    {
      var x: dynamic = 0;
      while ((x < a))
      {
        if ((sum <= war[x]))
        {
          co = x;
          break;
        }
        sum -= war[x];
        x += 1;
      }
    }
    write(d[co], "\n");
    cout.flush();
    var c: dynamic = cpp_uninitialized();
    read(c);
    {
      var x: dynamic = 0;
      while ((x < a))
      {
        if ((d[x] != c))
        {
          miss[x] += 1;
        }
        x += 1;
      }
    }
  }
  return 0;
}
