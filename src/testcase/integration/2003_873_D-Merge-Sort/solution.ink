// Translated from solution.cpp.

var arr = cpp_array(132005);

var n: dynamic;

var k: dynamic;

var init_cnt = 1;

var cnt = cpp_array((132005 << 1));

func init(l: dynamic, r: dynamic, p: dynamic)
{
  if (((r - l) <= 1))
  {
    return;
  }
  var mid = (((l + r)) >> 1);
  var lch = (p << 1);
  var rch = ((p << 1) | 1);
  init_cnt += 2;
  cnt[p] += 2;
  init(l, mid, lch);
  init(mid, r, rch);
  cnt[p] += (cnt[lch] + cnt[rch]);
}

var fix: dynamic;

func divide(l: dynamic, r: dynamic, p: dynamic)
{
  if (((r - l) <= 1))
  {
    return;
  }
  if ((cnt[p] <= fix))
  {
    sort((arr + l), (arr + r));
    fix -= cnt[p];
  } else
  {
    var mid = (((l + r)) >> 1);
    var lch = (p << 1);
    var rch = ((p << 1) | 1);
    if ((cnt[lch] <= fix))
    {
      sort((arr + l), (arr + mid));
      fix -= cnt[lch];
    } else
    {
      divide(l, mid, lch);
    }
    if ((cnt[rch] <= fix))
    {
      sort((arr + mid), (arr + r));
      fix -= cnt[rch];
    } else
    {
      divide(mid, r, rch);
    }
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, k);
  memset(cnt, 0, cpp_sizeof(cnt));
  init(0, n, 1);
  {
    var i = 0;
    while ((i < n))
    {
      arr[i] = (n - i);
      i += 1;
    }
  }
  if ((init_cnt < k))
  {
    write(-1, "\n");
  } else
  {
    fix = (init_cnt - k);
    divide(0, n, 1);
    if ((fix != 0))
    {
      write(-1, "\n");
    } else
    {
      {
        var i = 0;
        while ((i < n))
        {
          if ((i != 0))
          {
            write(" ");
          }
          write(arr[i]);
          i += 1;
        }
      }
      write("\n");
    }
  }
  return 0;
}
