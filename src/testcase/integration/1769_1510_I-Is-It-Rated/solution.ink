// Translated from solution.cpp.

var miss = cpp_array(1005);

var war = cpp_array(1005);

func losuj(a: dynamic)
{
  var res = 1;
  {
    var x = 1;
    while ((x <= 10))
    {
      res = ((((res * rand())) % a) + 1);
      x += 1;
    }
  }
  return res;
}

func main()
{
  srand((time(null) + clock()));
  var a: dynamic;
  var t: dynamic;
  read(a, t);
  while (cpp_update(t, "--"))
  {
    var d: dynamic;
    read(d);
    var mini = 1e9;
    var maks = -1e9;
    {
      var x = 0;
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
    var sum = 0;
    {
      var x = 0;
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
    var co = -1;
    {
      var x = 0;
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
    var c: dynamic;
    read(c);
    {
      var x = 0;
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
