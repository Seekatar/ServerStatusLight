using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Web;
using System.Web.Mvc;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace FakeApis.Controllers
{
    public class HomeController : Controller
    {
        public ActionResult Index()
        {
            ViewBag.Title = "Home Page";

            return View();
        }

        [HttpPost]
        public string ZabbixApi( string jsonrpc, string method, IDictionary<string,object> parms, int id, string auth )
        {
            HttpContext.Response.AppendHeader("Content-Type","application/json");
            if (method == "user.login")
            {
                return @"{
                        ""jsonrpc"":  ""2.0"",
                        ""result"":  ""8f55faf9c3773efc98c85f1083623aab"",
                        ""id"":  1
                    }";
            }
            else if (method == "event.get")
            {
                return @"{
                    ""jsonrpc"":  ""2.0"",
                    ""result"":  [
                                   {
                                       ""eventid"":  ""27752"",
                                       ""clock"":  ""1483555773"",
                                       ""objectid"":  ""13898"",
                                       ""r_eventid"":  ""234""
                                   },
      
                                   {
                                       ""eventid"":  ""23321"",
                                       ""clock"":  ""1483462655"",
                                       ""objectid"":  ""13758"",
                                       ""r_eventid"":  ""1234""
                                   },
                                   {
                                       ""eventid"":  ""23311"",
                                       ""clock"":  ""1483462535"",
                                       ""objectid"":  ""13726"",
                                       ""r_eventid"":  ""1234""
                                   },
                                   {
                                       ""eventid"":  ""23307"",
                                       ""clock"":  ""1483462481"",
                                       ""objectid"":  ""13801"",
                                       ""r_eventid"":  ""1234""
                                   },
                                   {
                                       ""eventid"":  ""23306"",
                                       ""clock"":  ""1483462475"",
                                       ""objectid"":  ""13892"",
                                       ""r_eventid"":  ""1243""
                                   },
                                   {
                                       ""eventid"":  ""23306"",
                                       ""clock"":  ""1483462475"",
                                       ""objectid"":  ""13726"",
                                       ""r_eventid"":  ""142""
                       
                                   }
                               ],
                    ""id"":  2
                }
                ";
            }
            else if (method == "trigger.get")
            {
                return @"{
                    ""jsonrpc"": ""2.0"",
                    ""result"": [
                        {
                            ""triggerid"": ""13898"",
                            ""priority"": ""1""
                        },
                        {
                            ""triggerid"": ""13758"",
                            ""priority"": ""2""
                        },
                        {
                            ""triggerid"": ""13726"",
                            ""priority"": ""3""
                        },
                        {
                            ""triggerid"": ""13801"",
                            ""priority"": ""4""
                        },
                        {
                            ""triggerid"": ""13892"",
                            ""priority"": ""5""
                        },
                        {
                            ""triggerid"": ""13884"",
                            ""priority"": ""4""
                        }
                    ],
                    ""id"": 2
                }";
            }
            else
            {
                return @"{""error"":""ow!""}";
            }
        }
    }
}
