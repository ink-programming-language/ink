// Translated from solution.cpp.

var MX = (1 << 10);

var n: dynamic;

var bitcnt = cpp_array((MX * 10));

var mp = cpp_array((MX * 10));

var ass = cpp_array((MX * 10));

var acnt: dynamic;

var ret = cpp_array(MX);

func main()
{
  read(n);
  {
    var i = 1;
    while ((i < ((1 << 13))))
    {
      bitcnt[i] = (1 + bitcnt[(i - ((i & (-i))))]);
      if ((bitcnt[i] == 6))
      {
        ass[cpp_update(acnt, "++")] = i;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      mp[ass[i]] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 13))
    {
      var cnt = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          if ((((((ass[j] >> i)) & 1)) == 0))
          {
            cnt += 1;
          }
          j += 1;
        }
      }
      if ((!cnt))
      {
        i += 1;
        continue;
      }
      write("? ", cnt, " ");
      {
        var j = 1;
        while ((j <= n))
        {
          if ((((((ass[j] >> i)) & 1)) == 0))
          {
            write(j, " ");
          }
          j += 1;
        }
      }
      write("\n");
      read(ret[i]);
      i += 1;
    }
  }
  write("! ");
  {
    var i = 1;
    while ((i <= n))
    {
      var Ans = 0;
      {
        var j = 0;
        while ((j < 13))
        {
          if ((((ass[i] >> j)) & 1))
          {
            Ans |= ret[j];
          }
          j += 1;
        }
      }
      write(Ans, " ");
      i += 1;
    }
  }
  write("\n");
}
