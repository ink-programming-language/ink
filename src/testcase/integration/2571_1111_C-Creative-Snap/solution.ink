// Translated from solution.cpp.

class node
{
  var no: dynamic;
  var cnt: dynamic;
  var len: dynamic;
  var prc: dynamic;
}

var n: dynamic;

var k: dynamic;

var a: dynamic;

var b: dynamic;

var plc = cpp_array(100005);

var s: dynamic;

var s1: dynamic;

func main()
{
  read(n, k, a, b);
  var mu = (1 << n);
  {
    var i = 1;
    while ((i <= k))
    {
      read(plc[i]);
      i += 1;
    }
  }
  sort((plc + 1), ((plc + k) + 1));
  var it = 0;
  plc[0] = plc[1];
  {
    var i = 1;
    while ((i <= k))
    {
      if ((plc[i] != plc[(i - 1)]))
      {
        s.emplace_back([((plc[(i - 1)] + mu) - 1), it, 1, (b * it)]);
        it = 1;
      } else
      {
        it += 1;
      }
      i += 1;
    }
  }
  s.emplace_back([((plc[k] + mu) - 1), it, 1, (b * it)]);
  while ((s[0].no != 1))
  {
    s1.clear();
    {
      var i = 0;
      while ((i < s.size()))
      {
        if (((i == (s.size() - 1)) || ((s[i].no / 2) != (s[(i + 1)].no / 2))))
        {
          s1.emplace_back([(s[i].no / 2), s[i].cnt, (s[i].len * 2), min((a + s[i].prc), (((s[i].cnt * b) * s[i].len) * 2))]);
        } else
        {
          s1.emplace_back([(s[i].no / 2), (s[i].cnt + s[(i + 1)].cnt), (s[i].len * 2), min((s[i].prc + s[(i + 1)].prc), (((((s[i].cnt + s[(i + 1)].cnt)) * b) * s[i].len) * 2))]);
          i += 1;
        }
        i += 1;
      }
    }
    s = s1;
  }
  write(s[0].prc);
}
